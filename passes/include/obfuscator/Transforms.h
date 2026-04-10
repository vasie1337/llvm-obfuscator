#pragma once

#include "llvm/IR/PassManager.h"

namespace obfuscator {

struct InstructionSubstitution
    : public llvm::PassInfoMixin<InstructionSubstitution> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};

} // namespace obfuscator
