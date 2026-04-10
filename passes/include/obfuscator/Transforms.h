#pragma once

#include "llvm/IR/PassManager.h"

namespace obfuscator {

struct InstructionSubstitution : public llvm::PassInfoMixin<InstructionSubstitution> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

struct ControlFlowFlattening : public llvm::PassInfoMixin<ControlFlowFlattening> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

struct MBASubstitution : public llvm::PassInfoMixin<MBASubstitution> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

struct BogusControlFlow : public llvm::PassInfoMixin<BogusControlFlow> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

struct StringEncryption : public llvm::PassInfoMixin<StringEncryption> {
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
};

struct SIMDObfuscation : public llvm::PassInfoMixin<SIMDObfuscation> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

struct ConstantUnfolding : public llvm::PassInfoMixin<ConstantUnfolding> {
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

struct BasicBlockFission : public llvm::PassInfoMixin<BasicBlockFission> {
    unsigned JunkPerFragment = 2;
    unsigned MaxJunkBlocks = 64;
    unsigned SplitThreshold = 1; // split every N instructions (1 = maximum splitting)

    BasicBlockFission() = default;
    BasicBlockFission(unsigned Junk, unsigned Max, unsigned Split)
        : JunkPerFragment(Junk), MaxJunkBlocks(Max), SplitThreshold(Split) {}

    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

} // namespace obfuscator
