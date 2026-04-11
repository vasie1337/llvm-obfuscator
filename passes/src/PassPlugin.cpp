#include "obfuscator/Transforms.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;
using namespace obfuscator;

static BasicBlockFission parseBBFissionParams(StringRef Params) {
    unsigned Junk = 2;
    unsigned Max = 64;
    unsigned Split = 1;
    while (!Params.empty()) {
        auto [Token, Rest] = Params.split(';');
        Params = Rest;
        auto [K, V] = Token.split('=');
        if (K == "junk") {
            V.getAsInteger(10, Junk);
        } else if (K == "max") {
            V.getAsInteger(10, Max);
        } else if (K == "split") {
            V.getAsInteger(10, Split);
        }
    }
    if (Split == 0) {
        Split = 1;
    }
    return {Junk, Max, Split};
}

llvm::PassPluginLibraryInfo getObfuscatorPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "obfuscator",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback([](StringRef Name, FunctionPassManager &FPM,
                                                  ArrayRef<PassBuilder::PipelineElement> Inner) {
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
                if (Name == "bbfission" || Name.starts_with("bbfission<")) {
                    StringRef Params;
                    if (Name.starts_with("bbfission<") && Name.ends_with(">")) {
                        Params = Name.slice(10, Name.size() - 1);
                    }
                    FPM.addPass(parseBBFissionParams(Params));
                    return true;
                }
                return false;
            });

            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "strenc") {
                        MPM.addPass(StringEncryption());
                        return true;
                    }
                    return false;
                });

            PB.registerPeepholeEPCallback([](FunctionPassManager &FPM, OptimizationLevel Level) {
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
                [](ModulePassManager &MPM, OptimizationLevel Level, ThinOrFullLTOPhase) {
                    if (Level != OptimizationLevel::O0) {
                        MPM.addPass(StringEncryption());
                    }
                });
        },
    };
}

// On MSVC, LLVM_ATTRIBUTE_WEAK is empty, so the entry point is not exported from the DLL and
// opt/clang cannot resolve llvmGetPassPluginInfo (legacy-plugin error).
#if defined(_MSC_VER)
#define LLVM_OBFUSCATOR_PLUGIN_ABI __declspec(dllexport)
#else
#define LLVM_OBFUSCATOR_PLUGIN_ABI LLVM_ATTRIBUTE_WEAK
#endif

extern "C" LLVM_OBFUSCATOR_PLUGIN_ABI ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getObfuscatorPluginInfo();
}
