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
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

} // namespace obfuscator
