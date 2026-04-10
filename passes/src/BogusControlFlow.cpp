#include "obfuscator/Transforms.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

namespace obfuscator {

static GlobalVariable *getOrCreateBCFGlobal(Module &M) {
  StringRef Name = ".bcf_opaque";
  if (auto *GV = M.getGlobalVariable(Name))
    return GV;
  auto *I32 = Type::getInt32Ty(M.getContext());
  return new GlobalVariable(M, I32, false, GlobalValue::PrivateLinkage,
                            ConstantInt::get(I32, 0), Name);
}

// Always-true predicate: (x*(x+1)) % 2 == 0 for any integer x.
// The volatile load prevents the optimizer from proving the value constant.
static Value *emitOpaquePredicate(IRBuilder<> &B, GlobalVariable *GV) {
  auto *X = B.CreateLoad(B.getInt32Ty(), GV, /*isVolatile=*/true);
  auto *XP1 = B.CreateAdd(X, B.getInt32(1));
  auto *Mul = B.CreateMul(X, XP1);
  auto *Rem = B.CreateURem(Mul, B.getInt32(2));
  return B.CreateICmpEQ(Rem, B.getInt32(0));
}

PreservedAnalyses BogusControlFlow::run(Function &F,
                                        FunctionAnalysisManager &AM) {
  if (F.isDeclaration() || F.empty())
    return PreservedAnalyses::all();

  auto *GV = getOrCreateBCFGlobal(*F.getParent());

  // Split the entry block so that the true entry (allocas, etc.) stays
  // predecessor-free while the body becomes a normal block we can target.
  BasicBlock &Entry = F.getEntryBlock();
  if (Entry.size() > 1)
    Entry.splitBasicBlock(Entry.getFirstNonPHIIt(), "bcf.entry");

  SmallVector<BasicBlock *, 16> Worklist;
  for (auto &BB : F)
    Worklist.push_back(&BB);

  bool Changed = false;

  for (auto *BB : Worklist) {
    // Never touch the real entry — LLVM requires it has no predecessors.
    if (BB == &F.getEntryBlock())
      continue;
    if (BB->size() <= 1)
      continue;
    if (isa<PHINode>(BB->front()))
      continue;

    auto *Term = BB->getTerminator();
    if (isa<InvokeInst>(Term) || isa<CatchSwitchInst>(Term) ||
        isa<CatchReturnInst>(Term) || isa<CleanupReturnInst>(Term))
      continue;

    // Split: BB becomes a tiny header; BodyBB gets all real instructions.
    BasicBlock *BodyBB =
        BB->splitBasicBlock(BB->getFirstNonPHIIt(), BB->getName() + ".body");

    // Clone BodyBB to produce an unreachable bogus copy.
    ValueToValueMapTy VMap;
    BasicBlock *BogusBB = CloneBasicBlock(BodyBB, VMap, ".bogus", &F);

    for (Instruction &I : *BogusBB)
      RemapInstruction(&I, VMap,
                       RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);

    // Replace the unconditional branch left by splitBasicBlock with an
    // opaque conditional: always-true → BodyBB, never-taken → BogusBB.
    BB->getTerminator()->eraseFromParent();
    IRBuilder<> Builder(BB);
    Value *Cond = emitOpaquePredicate(Builder, GV);
    Builder.CreateCondBr(Cond, BodyBB, BogusBB);

    // The bogus block loops back to the header, forming a dead cycle.
    BogusBB->getTerminator()->eraseFromParent();
    BranchInst::Create(BB, BogusBB);

    Changed = true;
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // namespace obfuscator
