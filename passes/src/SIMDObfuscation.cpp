#include "obfuscator/Transforms.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

#include <random>

using namespace llvm;

namespace obfuscator {

// Promote a scalar i32/i64 binary operation to an equivalent vector operation.
//
// For each scalar `OP i32 %a, %b`, we:
//   1. Build two <4 x i32> vectors with the real operands in a random lane
//      and noise (derived from the operands) in the other lanes.
//   2. Perform the same OP on the full vectors.
//   3. Shuffle the result one or two times.
//   4. Extract the correct element.
//
// In the final binary this produces SSE/AVX instructions (movd, pshufd,
// paddd, pxor, pextrd, …) that look like crypto or multimedia processing.
static Value *promoteToVector(BinaryOperator *BO, IRBuilder<> &Builder, std::mt19937 &RNG) {
    Value *A = BO->getOperand(0);
    Value *B = BO->getOperand(1);
    Type *ScalarTy = BO->getType();
    unsigned BitWidth = ScalarTy->getIntegerBitWidth();

    if (BitWidth != 32 && BitWidth != 64) {
        return nullptr;
    }

    unsigned NumElts = (BitWidth == 32) ? 4 : 2;
    auto *VecTy = FixedVectorType::get(ScalarTy, NumElts);

    auto RandConst = [&]() -> Value * {
        if (BitWidth == 32) {
            return ConstantInt::get(ScalarTy, RNG());
        }
        return ConstantInt::get(ScalarTy, (static_cast<uint64_t>(RNG()) << 32) | RNG());
    };

    unsigned RealLane = RNG() % NumElts;

    // Fill a vector with the real operand in RealLane and noise elsewhere.
    auto BuildVec = [&](Value *Real, Value *Other) -> Value * {
        Value *V = PoisonValue::get(VecTy);
        for (unsigned i = 0; i < NumElts; i++) {
            Value *Elt;
            if (i == RealLane) {
                Elt = Real;
            } else {
                switch (RNG() % 5) {
                case 0:
                    Elt = Builder.CreateXor(Real, RandConst());
                    break;
                case 1:
                    Elt = Builder.CreateAdd(Real, RandConst());
                    break;
                case 2:
                    Elt = Builder.CreateSub(Other, RandConst());
                    break;
                case 3:
                    Elt = Builder.CreateOr(Real, RandConst());
                    break;
                default:
                    Elt = Builder.CreateMul(Other, RandConst());
                    break;
                }
            }
            V = Builder.CreateInsertElement(V, Elt, Builder.getInt32(i));
        }
        return V;
    };

    Value *VA = BuildVec(A, B);
    Value *VB = BuildVec(B, A);

    Value *VResult;
    switch (BO->getOpcode()) {
    case Instruction::Add:
        VResult = Builder.CreateAdd(VA, VB);
        break;
    case Instruction::Sub:
        VResult = Builder.CreateSub(VA, VB);
        break;
    case Instruction::Mul:
        VResult = Builder.CreateMul(VA, VB);
        break;
    case Instruction::Xor:
        VResult = Builder.CreateXor(VA, VB);
        break;
    case Instruction::And:
        VResult = Builder.CreateAnd(VA, VB);
        break;
    case Instruction::Or:
        VResult = Builder.CreateOr(VA, VB);
        break;
    default:
        return nullptr;
    }

    // First shuffle — rotate the vector so the real element moves.
    unsigned Rotation = 1 + (RNG() % (NumElts - 1));
    SmallVector<int, 4> Mask(NumElts);
    for (unsigned i = 0; i < NumElts; i++) {
        Mask[i] = static_cast<int>((i + Rotation) % NumElts);
    }

    Value *Shuffled = Builder.CreateShuffleVector(VResult, Mask);
    // Mask[i] = (i+Rot)%N  ⇒  element at source position j lands at
    // result position (j − Rot + N) % N.
    unsigned ExtractLane = (RealLane - Rotation + NumElts) % NumElts;

    // Optionally apply a second shuffle for extra confusion.
    if (RNG() % 2 != 0) {
        unsigned Rot2 = 1 + (RNG() % (NumElts - 1));
        SmallVector<int, 4> Mask2(NumElts);
        for (unsigned i = 0; i < NumElts; i++) {
            Mask2[i] = static_cast<int>((i + Rot2) % NumElts);
        }
        Shuffled = Builder.CreateShuffleVector(Shuffled, Mask2);
        ExtractLane = (ExtractLane - Rot2 + NumElts) % NumElts;
    }

    return Builder.CreateExtractElement(Shuffled, Builder.getInt32(ExtractLane));
}

PreservedAnalyses SIMDObfuscation::run(Function &F, FunctionAnalysisManager &AM) {
    std::mt19937 RNG(std::hash<std::string>{}(F.getName().str()) ^ 0x53494D44);

    SmallVector<BinaryOperator *, 16> Targets;

    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            auto *BO = dyn_cast<BinaryOperator>(&I);
            if (BO == nullptr) {
                continue;
            }

            Type *Ty = BO->getType();
            if (!Ty->isIntegerTy(32) && !Ty->isIntegerTy(64)) {
                continue;
            }

            switch (BO->getOpcode()) {
            case Instruction::Add:
            case Instruction::Sub:
            case Instruction::Mul:
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
        if (Value *R = promoteToVector(BO, Builder, RNG)) {
            BO->replaceAllUsesWith(R);
            BO->eraseFromParent();
        }
    }

    return PreservedAnalyses::none();
}

} // namespace obfuscator
