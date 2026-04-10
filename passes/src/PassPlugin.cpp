#include "obfuscator/Transforms.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace obfuscator;

llvm::PassPluginLibraryInfo getObfuscatorPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Obfuscator", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // Register passes by name for: opt --passes="instsub"
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "instsub") {
                    FPM.addPass(InstructionSubstitution());
                    return true;
                  }
                  return false;
                });

            // Auto-register at O1+: clang -fpass-plugin=... -O1
            PB.registerPeepholeEPCallback(
                [](FunctionPassManager &FPM, OptimizationLevel Level) {
                  if (Level != OptimizationLevel::O0) {
                    FPM.addPass(InstructionSubstitution());
                  }
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getObfuscatorPluginInfo();
}
