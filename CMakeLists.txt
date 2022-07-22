cmake_minimum_required(VERSION 3.1.0)

project(KDToolBars)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(tests)
