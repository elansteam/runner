cmake_minimum_required(VERSION 3.22)
project(cpp-elans-safe-running)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
add_library(elans-safe-running src/runner.cpp)

add_executable(test main.cpp)
target_link_libraries(test elans-safe-running)

add_subdirectory(extern/pybind11)
pybind11_add_module(safe-running src/runner.cpp src/wrapper.cpp)