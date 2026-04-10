#include "obfuscator/Transforms.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

#include <random>

using namespace llvm;

namespace obfuscator {

using SubFn = Value *(*)(BinaryOperator *, IRBuilder<> &);

// ── Add: a + b ──────────────────────────────────────────────────────────────

// a + b  →  a - (-b)
static Value *addVar0(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateSub(A, B.CreateNeg(R));
}

// a + b  →  (a ^ b) + 2*(a & b)
static Value *addVar1(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  Value *Xor = B.CreateXor(A, R);
  Value *And = B.CreateAnd(A, R);
  return B.CreateAdd(Xor, B.CreateShl(And, 1));
}

// a + b  →  (a | b) + (a & b)
static Value *addVar2(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateAdd(B.CreateOr(A, R), B.CreateAnd(A, R));
}

// a + b  →  2*(a | b) - (a ^ b)
static Value *addVar3(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  Value *Or2 = B.CreateShl(B.CreateOr(A, R), 1);
  return B.CreateSub(Or2, B.CreateXor(A, R));
}

// a + b  →  ~(~a - b)
static Value *addVar4(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateNot(B.CreateSub(B.CreateNot(A), R));
}

// ── Sub: a - b ──────────────────────────────────────────────────────────────

// a - b  →  a + (-b)
static Value *subVar0(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateAdd(A, B.CreateNeg(R));
}

// a - b  →  (a ^ b) - 2*(~a & b)
static Value *subVar1(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  Value *Xor = B.CreateXor(A, R);
  Value *Mask = B.CreateAnd(B.CreateNot(A), R);
  return B.CreateSub(Xor, B.CreateShl(Mask, 1));
}

// a - b  →  ~(~a + b)
static Value *subVar2(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateNot(B.CreateAdd(B.CreateNot(A), R));
}

// a - b  →  2*(a & ~b) - (a ^ b)
static Value *subVar3(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  Value *Mask = B.CreateAnd(A, B.CreateNot(R));
  return B.CreateSub(B.CreateShl(Mask, 1), B.CreateXor(A, R));
}

// a - b  →  (a & ~b) - (~a & b)
static Value *subVar4(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  Value *L = B.CreateAnd(A, B.CreateNot(R));
  Value *Ri = B.CreateAnd(B.CreateNot(A), R);
  return B.CreateSub(L, Ri);
}

// ── Xor: a ^ b ──────────────────────────────────────────────────────────────

// a ^ b  →  (a | b) & ~(a & b)
static Value *xorVar0(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateAnd(B.CreateOr(A, R), B.CreateNot(B.CreateAnd(A, R)));
}

// a ^ b  →  (a & ~b) | (~a & b)
static Value *xorVar1(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  Value *L = B.CreateAnd(A, B.CreateNot(R));
  Value *Ri = B.CreateAnd(B.CreateNot(A), R);
  return B.CreateOr(L, Ri);
}

// a ^ b  →  (a | b) - (a & b)
static Value *xorVar2(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateSub(B.CreateOr(A, R), B.CreateAnd(A, R));
}

// a ^ b  →  (a + b) - 2*(a & b)
static Value *xorVar3(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateSub(B.CreateAdd(A, R), B.CreateShl(B.CreateAnd(A, R), 1));
}

// a ^ b  →  (~a & b) + (a & ~b)
static Value *xorVar4(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  Value *L = B.CreateAnd(B.CreateNot(A), R);
  Value *Ri = B.CreateAnd(A, B.CreateNot(R));
  return B.CreateAdd(L, Ri);
}

// ── And: a & b ──────────────────────────────────────────────────────────────

// a & b  →  ~(~a | ~b)
static Value *andVar0(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateNot(B.CreateOr(B.CreateNot(A), B.CreateNot(R)));
}

// a & b  →  (a + b) - (a | b)
static Value *andVar1(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateSub(B.CreateAdd(A, R), B.CreateOr(A, R));
}

// a & b  →  (a | b) ^ (a ^ b)
static Value *andVar2(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateXor(B.CreateOr(A, R), B.CreateXor(A, R));
}

// a & b  →  (a | b) & ~(a ^ b)
static Value *andVar3(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateAnd(B.CreateOr(A, R), B.CreateNot(B.CreateXor(A, R)));
}

// a & b  →  a - (a & ~b)     [subtract bits only in a]
static Value *andVar4(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateSub(A, B.CreateAnd(A, B.CreateNot(R)));
}

// ── Or: a | b ───────────────────────────────────────────────────────────────

// a | b  →  ~(~a & ~b)
static Value *orVar0(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateNot(B.CreateAnd(B.CreateNot(A), B.CreateNot(R)));
}

// a | b  →  (a ^ b) | (a & b)
static Value *orVar1(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateOr(B.CreateXor(A, R), B.CreateAnd(A, R));
}

// a | b  →  (a + b) - (a & b)
static Value *orVar2(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateSub(B.CreateAdd(A, R), B.CreateAnd(A, R));
}

// a | b  →  (a ^ b) + (a & b)
static Value *orVar3(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateAdd(B.CreateXor(A, R), B.CreateAnd(A, R));
}

// a | b  →  a + (b & ~a)     [add bits only in b]
static Value *orVar4(BinaryOperator *BO, IRBuilder<> &B) {
  Value *A = BO->getOperand(0), *R = BO->getOperand(1);
  return B.CreateAdd(A, B.CreateAnd(R, B.CreateNot(A)));
}

// ── Variant tables ──────────────────────────────────────────────────────────

static constexpr SubFn AddSubs[] = {addVar0, addVar1, addVar2, addVar3,
                                    addVar4};
static constexpr SubFn SubSubs[] = {subVar0, subVar1, subVar2, subVar3,
                                    subVar4};
static constexpr SubFn XorSubs[] = {xorVar0, xorVar1, xorVar2, xorVar3,
                                    xorVar4};
static constexpr SubFn AndSubs[] = {andVar0, andVar1, andVar2, andVar3,
                                    andVar4};
static constexpr SubFn OrSubs[]  = {orVar0, orVar1, orVar2, orVar3, orVar4};

static Value *pickAndApply(ArrayRef<SubFn> Variants, BinaryOperator *BO,
                           IRBuilder<> &Builder, std::mt19937 &RNG) {
  return Variants[RNG() % Variants.size()](BO, Builder);
}

// ── Pass entry ──────────────────────────────────────────────────────────────

PreservedAnalyses MBASubstitution::run(Function &F,
                                       FunctionAnalysisManager &AM) {
  std::mt19937 RNG(std::hash<std::string>{}(F.getName().str()));

  SmallVector<BinaryOperator *, 16> Targets;

  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      auto *BO = dyn_cast<BinaryOperator>(&I);
      if (!BO || !BO->getType()->isIntegerTy())
        continue;

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

  if (Targets.empty())
    return PreservedAnalyses::all();

  for (BinaryOperator *BO : Targets) {
    IRBuilder<> Builder(BO);
    Value *Replacement = nullptr;

    switch (BO->getOpcode()) {
    case Instruction::Add:
      Replacement = pickAndApply(AddSubs, BO, Builder, RNG);
      break;
    case Instruction::Sub:
      Replacement = pickAndApply(SubSubs, BO, Builder, RNG);
      break;
    case Instruction::Xor:
      Replacement = pickAndApply(XorSubs, BO, Builder, RNG);
      break;
    case Instruction::And:
      Replacement = pickAndApply(AndSubs, BO, Builder, RNG);
      break;
    case Instruction::Or:
      Replacement = pickAndApply(OrSubs, BO, Builder, RNG);
      break;
    default:
      break;
    }

    if (Replacement) {
      BO->replaceAllUsesWith(Replacement);
      BO->eraseFromParent();
    }
  }

  return PreservedAnalyses::none();
}

} // namespace obfuscator
