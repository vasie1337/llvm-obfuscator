#pragma once

#include "llvm/IR/PassManager.h"

namespace obfuscator {

struct InstructionSubstitution
    : public llvm::PassInfoMixin<InstructionSubstitution> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};

struct ControlFlowFlattening
    : public llvm::PassInfoMixin<ControlFlowFlattening> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};

struct MBASubstitution : public llvm::PassInfoMixin<MBASubstitution> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};

struct BogusControlFlow : public llvm::PassInfoMixin<BogusControlFlow> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};

} // namespace obfuscator
