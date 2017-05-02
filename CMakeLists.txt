cmake_minimum_required(VERSION 3.0)
project(ppmm VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ppmm
    MIPSCore.cpp
    main.cpp
    MIPSException.hpp
)
