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

    if(MSVC)
        target_compile_options(obfuscator PRIVATE /GR- /EHs-c- /utf-8)
    else()
        target_compile_options(obfuscator PRIVATE -fno-rtti -fno-exceptions)
        if(NOT WIN32)
            target_compile_options(obfuscator PRIVATE -fPIC)
        endif()
    endif()

    if(WIN32)
        set(_obf_suffix ".dll")
    elseif(APPLE)
        set(_obf_suffix ".dylib")
    else()
        set(_obf_suffix ".so")
    endif()
    set_target_properties(obfuscator PROPERTIES PREFIX "" SUFFIX "${_obf_suffix}")

    if(WIN32 AND MSVC)
        target_link_options(obfuscator PRIVATE "/FORCE:UNRESOLVED")
    elseif(WIN32)
        target_link_options(obfuscator PRIVATE "-Wl,--allow-shlib-undefined")
    elseif(APPLE)
        target_link_options(obfuscator PRIVATE
            "LINKER:-undefined,dynamic_lookup")
    elseif(UNIX)
        target_link_options(obfuscator PRIVATE
            "LINKER:--unresolved-symbols=ignore-in-object-files")
    endif()
else()
    add_llvm_pass_plugin(obfuscator ${OBFUSCATOR_SOURCES})
endif()
