cmake_minimum_required(VERSION 3.0)
project (qtppmm VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)


add_executable(qtppmm mainwindow.cpp main.cpp about.cpp images.qrc mainwindow.ui)

target_link_libraries(qtppmm mipscore mipsassembler)

qt5_use_modules(qtppmm Widgets)
