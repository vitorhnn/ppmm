cmake_minimum_required(VERSION 3.0)
project(ppmm VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)


add_executable(ppmm
    MIPSCore.cpp
    main.cpp
    MIPSException.hpp
    MIPSAssembler.hpp
    MIPSAssembler.cpp)

target_include_directories(ppmm PUBLIC ${Boost_INCLUDE_DIRS})
