find_package(Boost REQUIRED)

add_library(mipsassembler STATIC
    MIPSAssembler.cpp
    MIPSAssembler.hpp
)

target_include_directories(mipsassembler PUBLIC ${Boost_INCLUDE_DIRS})

