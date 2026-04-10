include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_llvm_pass_plugin(Obfuscator
    src/PassPlugin.cpp
    src/InstructionSubstitution.cpp
)
