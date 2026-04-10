#include "obfuscator/Transforms.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace obfuscator;

llvm::PassPluginLibraryInfo getObfuscatorPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "obfuscator", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // Register function passes by name for: opt --passes="instsub"
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "instsub") {
                    FPM.addPass(InstructionSubstitution());
                    return true;
                  }
                  if (Name == "cff") {
                    FPM.addPass(ControlFlowFlattening());
                    return true;
                  }
                  if (Name == "mbasub") {
                    FPM.addPass(MBASubstitution());
                    return true;
                  }
                  if (Name == "bcf") {
                    FPM.addPass(BogusControlFlow());
                    return true;
                  }
                  if (Name == "simd") {
                    FPM.addPass(SIMDObfuscation());
                    return true;
                  }
                  if (Name == "constunfold") {
                    FPM.addPass(ConstantUnfolding());
                    return true;
                  }
                  return false;
                });

            // Register module passes by name for: opt --passes="strenc"
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "strenc") {
                    MPM.addPass(StringEncryption());
                    return true;
                  }
                  return false;
                });

            // Auto-register at O1+: clang -fpass-plugin=... -O1
            PB.registerPeepholeEPCallback(
                [](FunctionPassManager &FPM, OptimizationLevel Level) {
                  if (Level != OptimizationLevel::O0) {
                    FPM.addPass(InstructionSubstitution());
                    FPM.addPass(MBASubstitution());
                  }
                });

            // SIMD + constant-unfolding run before CFF so their new
            // instructions are placed while the CFG is still normal.
            // CFF restructures the entire CFG (dispatcher loop), so it
            // must be the very last function-level transform.
            PB.registerScalarOptimizerLateEPCallback(
                [](FunctionPassManager &FPM, OptimizationLevel Level) {
                  if (Level != OptimizationLevel::O0) {
                    FPM.addPass(SIMDObfuscation());
                    FPM.addPass(ConstantUnfolding());
                    FPM.addPass(ControlFlowFlattening());
                  }
                });

            // String encryption runs last so it sees the final set of
            // string literals after all other optimizations.
            PB.registerOptimizerLastEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level,
                   ThinOrFullLTOPhase) {
                  if (Level != OptimizationLevel::O0)
                    MPM.addPass(StringEncryption());
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getObfuscatorPluginInfo();
}
