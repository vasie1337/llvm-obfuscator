include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(OBFUSCATOR_SOURCES
    src/PassPlugin.cpp
    src/InstructionSubstitution.cpp
    src/MBASubstitution.cpp
    src/ControlFlowFlattening.cpp
    src/BogusControlFlow.cpp
    src/StringEncryption.cpp
    src/SIMDObfuscation.cpp
    src/ConstantUnfolding.cpp
    src/BasicBlockFission.cpp
)

option(OBFUSCATOR_STANDALONE_MODULE
    "Build as a standalone MODULE that does NOT link libLLVM (for use with \
     hosts that bundle their own LLVM, e.g. rustc). Headers-only build."
    OFF)

if(OBFUSCATOR_STANDALONE_MODULE)
    add_library(obfuscator MODULE ${OBFUSCATOR_SOURCES})
    target_include_directories(obfuscator PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(obfuscator PRIVATE ${LLVM_DEFINITIONS_LIST})
    target_compile_features(obfuscator PRIVATE cxx_std_17)
    target_compile_options(obfuscator PRIVATE -fno-rtti -fno-exceptions -fPIC)
    set_target_properties(obfuscator PROPERTIES PREFIX "" SUFFIX ".so")
    if(UNIX AND NOT APPLE)
        target_link_options(obfuscator PRIVATE
            "LINKER:--unresolved-symbols=ignore-in-object-files")
    elseif(APPLE)
        target_link_options(obfuscator PRIVATE
            "LINKER:-undefined,dynamic_lookup")
    endif()
else()
    if(WIN32)
        add_library(obfuscator SHARED ${OBFUSCATOR_SOURCES})
        target_include_directories(obfuscator PRIVATE ${LLVM_INCLUDE_DIRS})
        target_compile_definitions(obfuscator PRIVATE ${LLVM_DEFINITIONS_LIST})
        target_compile_features(obfuscator PRIVATE cxx_std_17)

        # Loading this DLL into clang-cl with a *second* full static LLVM inside
        # the plugin causes UB (often 0xC000001D). The supported layout is one
        # LLVM in the process: link only the shared LLVM import lib when available.
        if(LLVM_LINK_LLVM_DYLIB AND (TARGET LLVM OR EXISTS "${LLVM_LIBRARY_DIR}/LLVM.lib"))
            if(TARGET LLVM)
                target_link_libraries(obfuscator PRIVATE LLVM)
            else()
                target_link_libraries(obfuscator PRIVATE "${LLVM_LIBRARY_DIR}/LLVM.lib")
            endif()
            target_link_libraries(obfuscator PRIVATE ntdll advapi32 shell32 ole32)
        else()
            message(WARNING
                "Windows: LLVM was built without a shared libLLVM (LLVM_LINK_LLVM_DYLIB=OFF). "
                "The plugin is linked against static LLVM .libs; loading it with "
                "clang -fpass-plugin typically crashes. Rebuild LLVM with "
                "-DLLVM_BUILD_LLVM_DYLIB=ON (and use the resulting LLVM.lib/LLVM.dll), "
                "or use Linux/WSL for -fpass-plugin.")
            set(_llvm_libs
                LLVMCore LLVMSupport LLVMPasses LLVMAnalysis LLVMTransformUtils
                LLVMScalarOpts LLVMipo LLVMInstCombine LLVMAggressiveInstCombine
                LLVMVectorize LLVMTarget LLVMLinker LLVMBinaryFormat
                LLVMBitReader LLVMBitWriter LLVMMC LLVMMCParser LLVMObject
                LLVMProfileData LLVMRemarks LLVMCodeGen LLVMCodeGenTypes
                LLVMIRReader LLVMAsmParser LLVMIRPrinter LLVMDemangle
                LLVMTargetParser LLVMDebugInfoCodeView LLVMDebugInfoDWARF
                LLVMDebugInfoMSF LLVMDebugInfoPDB LLVMTextAPI
                LLVMCoroutines LLVMCFGuard LLVMExtensions LLVMOption
                LLVMFrontendOpenMP LLVMFrontendOffloading LLVMFrontendDriver
                LLVMFrontendHLSL LLVMFrontendAtomic LLVMHipStdPar
                LLVMObjCARCOpts LLVMInstrumentation LLVMWindowsManifest
                LLVMWindowsDriver LLVMSandboxIR LLVMTelemetry
                LLVMSelectionDAG LLVMGlobalISel LLVMAsmPrinter
                LLVMBitstreamReader LLVMCGData
            )
            set(_link_libs "")
            foreach(_lib ${_llvm_libs})
                list(APPEND _link_libs "${LLVM_LIBRARY_DIR}/${_lib}.lib")
            endforeach()

            set(_dia_lib "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/DIA SDK/lib/amd64/diaguids.lib")
            if(EXISTS "${_dia_lib}")
                list(APPEND _link_libs "${_dia_lib}")
            endif()

            list(APPEND _link_libs ntdll advapi32 shell32 ole32)

            target_link_libraries(obfuscator PRIVATE ${_link_libs})
        endif()

        # Export the plugin entry point without using __declspec(dllexport) on
        # llvmGetPassPluginInfo (that conflicts with PassPlugin.h's declaration).
        if(MSVC)
            target_link_options(obfuscator PRIVATE "/EXPORT:llvmGetPassPluginInfo")
        endif()

        set_target_properties(obfuscator PROPERTIES PREFIX "" SUFFIX ".dll")
    else()
        add_llvm_pass_plugin(obfuscator ${OBFUSCATOR_SOURCES})
    endif()
endif()
