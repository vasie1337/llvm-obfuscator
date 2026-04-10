#include "obfuscator/Transforms.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace obfuscator {

// Substitute: a + b  →  a - (-b)
static Value *substituteAdd(BinaryOperator *BO, IRBuilder<> &Builder) {
    Value *A = BO->getOperand(0);
    Value *B = BO->getOperand(1);
    Value *NegB = Builder.CreateNeg(B);
    return Builder.CreateSub(A, NegB);
}

// Substitute: a - b  →  a + (-b)
static Value *substituteSub(BinaryOperator *BO, IRBuilder<> &Builder) {
    Value *A = BO->getOperand(0);
    Value *B = BO->getOperand(1);
    Value *NegB = Builder.CreateNeg(B);
    return Builder.CreateAdd(A, NegB);
}

// Substitute: a ^ b  →  (a | b) & ~(a & b)
static Value *substituteXor(BinaryOperator *BO, IRBuilder<> &Builder) {
    Value *A = BO->getOperand(0);
    Value *B = BO->getOperand(1);
    Value *OrAB = Builder.CreateOr(A, B);
    Value *AndAB = Builder.CreateAnd(A, B);
    Value *NotAndAB = Builder.CreateNot(AndAB);
    return Builder.CreateAnd(OrAB, NotAndAB);
}

// Substitute: a & b  →  (a ^ ~b) & a    [identity: only bits set in both]
//             simpler: ~(~a | ~b)
static Value *substituteAnd(BinaryOperator *BO, IRBuilder<> &Builder) {
    Value *A = BO->getOperand(0);
    Value *B = BO->getOperand(1);
    Value *NotA = Builder.CreateNot(A);
    Value *NotB = Builder.CreateNot(B);
    Value *OrNotANotB = Builder.CreateOr(NotA, NotB);
    return Builder.CreateNot(OrNotANotB);
}

// Substitute: a | b  →  (a & ~b) | b    ... simpler: ~(~a & ~b)
static Value *substituteOr(BinaryOperator *BO, IRBuilder<> &Builder) {
    Value *A = BO->getOperand(0);
    Value *B = BO->getOperand(1);
    Value *NotA = Builder.CreateNot(A);
    Value *NotB = Builder.CreateNot(B);
    Value *AndNotANotB = Builder.CreateAnd(NotA, NotB);
    return Builder.CreateNot(AndNotANotB);
}

PreservedAnalyses InstructionSubstitution::run(Function &F, FunctionAnalysisManager &AM) {
    SmallVector<BinaryOperator *, 16> Targets;

    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            auto *BO = dyn_cast<BinaryOperator>(&I);
            if (BO == nullptr || !BO->getType()->isIntegerTy()) {
                continue;
            }

            switch (BO->getOpcode()) {
            case Instruction::Add:
            case Instruction::Sub:
            case Instruction::Xor:
            case Instruction::And:
            case Instruction::Or:
                Targets.push_back(BO);
                break;
            default:
                break;
            }
        }
    }

    if (Targets.empty()) {
        return PreservedAnalyses::all();
    }

    for (BinaryOperator *BO : Targets) {
        IRBuilder<> Builder(BO);
        Value *Replacement = nullptr;

        switch (BO->getOpcode()) {
        case Instruction::Add:
            Replacement = substituteAdd(BO, Builder);
            break;
        case Instruction::Sub:
            Replacement = substituteSub(BO, Builder);
            break;
        case Instruction::Xor:
            Replacement = substituteXor(BO, Builder);
            break;
        case Instruction::And:
            Replacement = substituteAnd(BO, Builder);
            break;
        case Instruction::Or:
            Replacement = substituteOr(BO, Builder);
            break;
        default:
            break;
        }

        if (Replacement != nullptr) {
            BO->replaceAllUsesWith(Replacement);
            BO->eraseFromParent();
        }
    }

    return PreservedAnalyses::none();
}

} // namespace obfuscator
