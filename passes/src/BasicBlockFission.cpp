#include "obfuscator/Transforms.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Local.h"

#include <random>

using namespace llvm;

namespace obfuscator {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void demoteRegisters(Function &F) {
    SmallVector<PHINode *, 16> Phis;
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (auto *PN = dyn_cast<PHINode>(&I)) {
                Phis.push_back(PN);
            }
        }
    }
    for (PHINode *PN : Phis) {
        DemotePHIToStack(PN);
    }

    SmallVector<Instruction *, 16> Sinks;
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (isa<AllocaInst>(&I)) {
                continue;
            }
            for (Use &U : I.uses()) {
                auto *User = cast<Instruction>(U.getUser());
                if (User->getParent() != &BB) {
                    Sinks.push_back(&I);
                    break;
                }
            }
        }
    }
    for (Instruction *I : Sinks) {
        DemoteRegToStack(*I);
    }
}

static GlobalVariable *getOrCreateOpaqueGlobal(Module &M, StringRef Name) {
    if (auto *GV = M.getGlobalVariable(Name)) {
        return GV;
    }
    auto *I32 = Type::getInt32Ty(M.getContext());
    return new GlobalVariable(M, I32, false, GlobalValue::PrivateLinkage, ConstantInt::get(I32, 0),
                              Name);
}

// (x * (x + 1)) % 2 == 0  — always true for any integer x.
static Value *emitAlwaysTrue(IRBuilder<> &B, GlobalVariable *GV) {
    auto *X = B.CreateLoad(B.getInt32Ty(), GV, /*isVolatile=*/true);
    auto *XP1 = B.CreateAdd(X, B.getInt32(1));
    auto *Mul = B.CreateMul(X, XP1);
    auto *Rem = B.CreateURem(Mul, B.getInt32(2));
    return B.CreateICmpEQ(Rem, B.getInt32(0));
}

// (x^2 + x + 7) % 2 == 0  — always false (x^2+x is always even, +7 is odd).
static Value *emitAlwaysFalse(IRBuilder<> &B, GlobalVariable *GV) {
    auto *X = B.CreateLoad(B.getInt32Ty(), GV, /*isVolatile=*/true);
    auto *X2 = B.CreateMul(X, X);
    auto *Sum = B.CreateAdd(X2, X);
    auto *Plus7 = B.CreateAdd(Sum, B.getInt32(7));
    auto *Rem = B.CreateURem(Plus7, B.getInt32(2));
    return B.CreateICmpEQ(Rem, B.getInt32(0));
}

// ---------------------------------------------------------------------------
// Pass entry
// ---------------------------------------------------------------------------

PreservedAnalyses BasicBlockFission::run(Function &F, FunctionAnalysisManager &AM) {
    if (F.isDeclaration() || F.empty()) {
        return PreservedAnalyses::all();
    }

    Module &M = *F.getParent();
    auto *GV1 = getOrCreateOpaqueGlobal(M, ".bbf_opaque_a");
    auto *GV2 = getOrCreateOpaqueGlobal(M, ".bbf_opaque_b");

    std::mt19937 Rng(0x46495353); // NOLINT(bugprone-random-generator-seed)

    // -----------------------------------------------------------------------
    // Phase 0 — Split the entry block so allocas stay in the true entry and
    //           the rest becomes a normal body block we can fragment.
    // -----------------------------------------------------------------------
    BasicBlock *EntryBB = &F.getEntryBlock();
    auto SplitPt = EntryBB->begin();
    while (SplitPt != EntryBB->end() && (isa<AllocaInst>(&*SplitPt) || isa<PHINode>(&*SplitPt))) {
        ++SplitPt;
    }
    if (SplitPt == EntryBB->end() || SplitPt->isTerminator()) {
        return PreservedAnalyses::all();
    }
    EntryBB->splitBasicBlock(&*SplitPt, "bbf.body");

    // -----------------------------------------------------------------------
    // Phase 1 — Split every non-entry basic block into tiny fragments
    //           (1-2 instructions each).
    // -----------------------------------------------------------------------
    SmallVector<BasicBlock *, 64> Fragments;
    {
        SmallVector<BasicBlock *, 32> OrigBlocks;
        for (auto &BB : F) {
            OrigBlocks.push_back(&BB);
        }

        for (auto *BB : OrigBlocks) {
            if (BB == &F.getEntryBlock()) {
                continue;
            }

            SmallVector<Instruction *, 32> Insts;
            for (auto &I : *BB) {
                if (isa<PHINode>(&I)) {
                    continue;
                }
                Insts.push_back(&I);
            }

            if (Insts.size() <= 2) {
                Fragments.push_back(BB);
                continue;
            }

            BasicBlock *Cur = BB;
            unsigned Count = 0;
            for (unsigned Idx = 0; Idx < Insts.size() - 1; ++Idx) {
                Count++;
                unsigned Threshold = 1 + (Rng() % 2);
                if (Count >= Threshold) {
                    auto It = std::next(Insts[Idx]->getIterator());
                    if (It != Cur->end() && !isa<BranchInst>(&*It) && !isa<ReturnInst>(&*It) &&
                        !isa<SwitchInst>(&*It) && !isa<UnreachableInst>(&*It)) {
                        BasicBlock *Next =
                            Cur->splitBasicBlock(&*It, "bbf.frag." + Twine(Rng() % 99999));
                        Fragments.push_back(Cur);
                        Cur = Next;
                        Count = 0;
                    }
                }
            }
            Fragments.push_back(Cur);
        }
    }

    if (Fragments.size() < 2) {
        return PreservedAnalyses::all();
    }

    demoteRegisters(F);

    // -----------------------------------------------------------------------
    // Phase 2 — Create a mesh of junk blocks.
    // Each junk block contains dead arithmetic and opaque-predicate branches
    // into other junk blocks (or occasionally real fragments), producing a
    // dense tangled CFG web.
    // -----------------------------------------------------------------------
    constexpr unsigned JunkPerFragment = 2;
    unsigned NumJunk = Fragments.size() * JunkPerFragment;
    SmallVector<BasicBlock *, 64> JunkBlocks;
    JunkBlocks.reserve(NumJunk);

    for (unsigned I = 0; I < NumJunk; ++I) {
        BasicBlock *JBB =
            BasicBlock::Create(F.getContext(), "bbf.junk." + Twine(Rng() % 99999), &F);
        IRBuilder<> JB(JBB);

        auto *V = JB.CreateLoad(JB.getInt32Ty(), GV1, /*isVolatile=*/true);
        auto *A = JB.CreateXor(V, JB.getInt32(Rng()));
        auto *B = JB.CreateAdd(A, JB.getInt32(Rng()));
        auto *C = JB.CreateMul(B, JB.getInt32(Rng() | 1));
        JB.CreateStore(C, GV2, /*isVolatile=*/true);

        JB.CreateUnreachable();
        JunkBlocks.push_back(JBB);
    }

    // Wire junk blocks into a tangled mesh.
    for (unsigned I = 0; I < JunkBlocks.size(); ++I) {
        BasicBlock *JBB = JunkBlocks[I];
        JBB->getTerminator()->eraseFromParent();
        IRBuilder<> JB(JBB);

        unsigned TgtA = Rng() % JunkBlocks.size();
        unsigned TgtB = Rng() % JunkBlocks.size();
        BasicBlock *TrueDest = JunkBlocks[TgtA];
        BasicBlock *FalseDest = JunkBlocks[TgtB];

        if (Rng() % 3 == 0) {
            TrueDest = Fragments[Rng() % Fragments.size()];
        }
        if (Rng() % 3 == 0) {
            FalseDest = Fragments[Rng() % Fragments.size()];
        }

        Value *Cond = (Rng() % 2 == 0) ? emitAlwaysTrue(JB, GV1) : emitAlwaysFalse(JB, GV2);
        JB.CreateCondBr(Cond, TrueDest, FalseDest);
    }

    // -----------------------------------------------------------------------
    // Phase 3 — Replace unconditional edges between fragments with
    //           opaque-predicate branches through junk blocks.
    // -----------------------------------------------------------------------
    if (JunkBlocks.empty()) {
        return PreservedAnalyses::none();
    }

    unsigned JunkIdx = 0;
    for (auto *Frag : Fragments) {
        if (Frag == &F.getEntryBlock()) {
            continue;
        }

        auto *Term = Frag->getTerminator();
        if (Term == nullptr || !isa<BranchInst>(Term)) {
            continue;
        }
        auto *Br = cast<BranchInst>(Term);
        if (!Br->isUnconditional()) {
            continue;
        }

        BasicBlock *RealSucc = Br->getSuccessor(0);
        BasicBlock *Junk = JunkBlocks[JunkIdx % JunkBlocks.size()];
        JunkIdx++;

        Br->eraseFromParent();
        IRBuilder<> Builder(Frag);
        Value *Cond = emitAlwaysTrue(Builder, GV1);
        Builder.CreateCondBr(Cond, RealSucc, Junk);
    }

    return PreservedAnalyses::none();
}

} // namespace obfuscator
