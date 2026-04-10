#include "obfuscator/Transforms.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include <random>

using namespace llvm;

namespace obfuscator {

// Per-bit-width volatile global initialized to zero.  A volatile load
// prevents the optimizer from proving the value is zero, so the
// arithmetic that reconstructs the original constant cannot be folded.
static GlobalVariable *getOrCreateOpaqueZero(Module &M, unsigned BitWidth) {
  std::string Name = ".constunfold_" + std::to_string(BitWidth);
  if (auto *GV = M.getGlobalVariable(Name))
    return GV;
  auto *Ty = IntegerType::get(M.getContext(), BitWidth);
  return new GlobalVariable(M, Ty, /*isConstant=*/false,
                            GlobalValue::PrivateLinkage,
                            Constant::getNullValue(Ty), Name);
}

PreservedAnalyses ConstantUnfolding::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  Module &M = *F.getParent();
  std::mt19937 RNG(std::hash<std::string>{}(F.getName().str()) ^ 0x554E464F);

  struct Target {
    Instruction *I;
    unsigned OpIdx;
    ConstantInt *CI;
  };
  SmallVector<Target, 32> Targets;

  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      // Only unfold constants inside binary ops and integer comparisons —
      // these are the immediates that reveal algorithm structure.
      if (!isa<BinaryOperator>(I) && !isa<ICmpInst>(I))
        continue;
      if (isa<PHINode>(I))
        continue;

      for (unsigned i = 0, e = I.getNumOperands(); i < e; i++) {
        auto *CI = dyn_cast<ConstantInt>(I.getOperand(i));
        if (!CI)
          continue;
        unsigned BW = CI->getBitWidth();
        if (BW < 16)
          continue;
        int64_t Val = CI->getSExtValue();
        if (Val == 0 || Val == 1 || Val == -1)
          continue;
        Targets.push_back({&I, i, CI});
      }
    }
  }

  if (Targets.empty())
    return PreservedAnalyses::all();

  DenseMap<unsigned, GlobalVariable *> Zeros;
  auto getZero = [&](unsigned BW) -> GlobalVariable * {
    auto It = Zeros.find(BW);
    if (It != Zeros.end())
      return It->second;
    auto *GV = getOrCreateOpaqueZero(M, BW);
    Zeros[BW] = GV;
    return GV;
  };

  auto RandKey = [&](unsigned BW) -> uint64_t {
    uint64_t K = RNG();
    if (BW > 32)
      K = (K << 32) | RNG();
    return K;
  };

  for (auto &T : Targets) {
    IRBuilder<> Builder(T.I);
    Type *Ty = T.CI->getType();
    unsigned BW = T.CI->getBitWidth();
    uint64_t Orig = T.CI->getZExtValue();

    Value *OpaqueZ =
        Builder.CreateLoad(Ty, getZero(BW), /*isVolatile=*/true);

    Value *Result;
    switch (RNG() % 4) {
    case 0: {
      // C = (opaque + (C+K)) - K
      uint64_t K = RandKey(BW);
      Value *Sum = Builder.CreateAdd(
          OpaqueZ, ConstantInt::get(Ty, Orig + K));
      Result = Builder.CreateSub(Sum, ConstantInt::get(Ty, K));
      break;
    }
    case 1: {
      // C = (opaque ^ KEY) ^ (C ^ KEY)
      uint64_t K = RandKey(BW);
      Value *X1 = Builder.CreateXor(OpaqueZ, ConstantInt::get(Ty, K));
      Result = Builder.CreateXor(X1, ConstantInt::get(Ty, Orig ^ K));
      break;
    }
    case 2: {
      // C = ~(opaque ^ ~C)
      Value *X = Builder.CreateXor(OpaqueZ, ConstantInt::get(Ty, ~Orig));
      Result = Builder.CreateNot(X);
      break;
    }
    default: {
      // C = ((opaque - K1) + K1) + C   (two-step identity with add)
      uint64_t K1 = RandKey(BW);
      Value *S = Builder.CreateSub(OpaqueZ, ConstantInt::get(Ty, K1));
      Value *A = Builder.CreateAdd(S, ConstantInt::get(Ty, K1));
      Result = Builder.CreateAdd(A, ConstantInt::get(Ty, Orig));
      break;
    }
    }

    T.I->setOperand(T.OpIdx, Result);
  }

  return PreservedAnalyses::none();
}

} // namespace obfuscator
