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
    add_llvm_pass_plugin(obfuscator ${OBFUSCATOR_SOURCES})
endif()
