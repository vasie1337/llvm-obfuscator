#include "obfuscator/Transforms.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/Local.h"

#include <random>

using namespace llvm;

namespace obfuscator {

// Demote all PHI nodes and register-based values used across basic blocks to
// stack allocas so that blocks can be reordered freely by the dispatcher.
static void demoteRegisters(Function &F) {
  SmallVector<PHINode *, 16> Phis;
  SmallVector<Instruction *, 16> Sinks;

  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (auto *PN = dyn_cast<PHINode>(&I)) {
        Phis.push_back(PN);
      } else {
        for (Use &U : I.uses()) {
          auto *User = cast<Instruction>(U.getUser());
          if (User->getParent() != &BB) {
            Sinks.push_back(&I);
            break;
          }
        }
      }
    }
  }

  for (PHINode *PN : Phis)
    DemotePHIToStack(PN);

  for (Instruction *I : Sinks)
    DemoteRegToStack(*I);
}

PreservedAnalyses ControlFlowFlattening::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  if (F.isDeclaration() || F.size() <= 1)
    return PreservedAnalyses::all();

  demoteRegisters(F);

  SmallVector<BasicBlock *, 16> OrigBlocks;
  for (BasicBlock &BB : F)
    OrigBlocks.push_back(&BB);

  // The original entry block must stay first; split it so the preamble
  // (allocas, etc.) remains in the true entry and the rest becomes a normal
  // case block that the dispatcher can target.
  BasicBlock *EntryBB = OrigBlocks[0];
  BasicBlock *FirstBody =
      EntryBB->splitBasicBlock(EntryBB->getFirstNonPHIOrDbgOrLifetime());

  // Remove the old entry from the list and replace with the split-off body.
  OrigBlocks[0] = FirstBody;

  // Assign random 32-bit IDs to each block.
  std::mt19937 Rng(0x4f425343); // fixed seed for reproducibility
  std::uniform_int_distribution<int32_t> Dist;
  SmallVector<int32_t, 16> BlockIDs;
  for (size_t I = 0; I < OrigBlocks.size(); ++I)
    BlockIDs.push_back(Dist(Rng));

  // Create a switch variable in the entry block.
  IRBuilder<> EntryBuilder(EntryBB->getTerminator());
  AllocaInst *SwitchVar =
      EntryBuilder.CreateAlloca(EntryBuilder.getInt32Ty(), nullptr);
  EntryBuilder.CreateStore(EntryBuilder.getInt32(BlockIDs[0]), SwitchVar);

  // Replace the entry's unconditional branch with a jump to the dispatcher.
  BasicBlock *DispatchBB = BasicBlock::Create(F.getContext(), "cff.dispatch",
                                              &F, FirstBody);
  EntryBB->getTerminator()->eraseFromParent();
  BranchInst::Create(DispatchBB, EntryBB);

  // Build the dispatcher: load switch var and jump to the matching case.
  IRBuilder<> DispBuilder(DispatchBB);
  LoadInst *SwitchLoad = DispBuilder.CreateLoad(DispBuilder.getInt32Ty(),
                                                SwitchVar);
  BasicBlock *DefaultBB = BasicBlock::Create(F.getContext(), "cff.default", &F);
  SwitchInst *Dispatch =
      DispBuilder.CreateSwitch(SwitchLoad, DefaultBB, OrigBlocks.size());

  // Default block just loops back to the dispatcher (should never be reached).
  BranchInst::Create(DispatchBB, DefaultBB);

  // Wire each original block as a switch case.
  for (size_t I = 0; I < OrigBlocks.size(); ++I)
    Dispatch->addCase(DispBuilder.getInt32(BlockIDs[I]), OrigBlocks[I]);

  // Rewrite every block's terminator to go through the dispatcher.
  for (size_t I = 0; I < OrigBlocks.size(); ++I) {
    BasicBlock *BB = OrigBlocks[I];
    Instruction *Term = BB->getTerminator();

    if (isa<ReturnInst>(Term) || isa<UnreachableInst>(Term))
      continue;

    if (auto *Br = dyn_cast<BranchInst>(Term)) {
      IRBuilder<> Builder(Term);

      if (Br->isUnconditional()) {
        BasicBlock *Succ = Br->getSuccessor(0);
        int32_t SuccID = 0;
        for (size_t J = 0; J < OrigBlocks.size(); ++J)
          if (OrigBlocks[J] == Succ) { SuccID = BlockIDs[J]; break; }

        Builder.CreateStore(Builder.getInt32(SuccID), SwitchVar);
        Builder.CreateBr(DispatchBB);
        Term->eraseFromParent();
      } else {
        BasicBlock *TrueBB = Br->getSuccessor(0);
        BasicBlock *FalseBB = Br->getSuccessor(1);
        Value *Cond = Br->getCondition();

        int32_t TrueID = 0, FalseID = 0;
        for (size_t J = 0; J < OrigBlocks.size(); ++J) {
          if (OrigBlocks[J] == TrueBB)  TrueID  = BlockIDs[J];
          if (OrigBlocks[J] == FalseBB) FalseID = BlockIDs[J];
        }

        Value *SelID = Builder.CreateSelect(
            Cond, Builder.getInt32(TrueID), Builder.getInt32(FalseID));
        Builder.CreateStore(SelID, SwitchVar);
        Builder.CreateBr(DispatchBB);
        Term->eraseFromParent();
      }
    } else if (auto *SI = dyn_cast<SwitchInst>(Term)) {
      IRBuilder<> Builder(Term);

      // Map the default destination.
      BasicBlock *DefDest = SI->getDefaultDest();
      int32_t DefID = 0;
      for (size_t J = 0; J < OrigBlocks.size(); ++J)
        if (OrigBlocks[J] == DefDest) { DefID = BlockIDs[J]; break; }

      Value *Result = Builder.getInt32(DefID);

      for (auto Case : SI->cases()) {
        BasicBlock *CaseDest = Case.getCaseSuccessor();
        int32_t CaseID = 0;
        for (size_t J = 0; J < OrigBlocks.size(); ++J)
          if (OrigBlocks[J] == CaseDest) { CaseID = BlockIDs[J]; break; }

        Value *Cmp = Builder.CreateICmpEQ(SI->getCondition(),
                                          Case.getCaseValue());
        Result = Builder.CreateSelect(Cmp, Builder.getInt32(CaseID), Result);
      }

      Builder.CreateStore(Result, SwitchVar);
      Builder.CreateBr(DispatchBB);
      Term->eraseFromParent();
    }
  }

  return PreservedAnalyses::none();
}

} // namespace obfuscator
