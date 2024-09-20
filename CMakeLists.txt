cmake_minimum_required(VERSION 3.25)
project(cpp-elans-runner)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_CPPLINT "cpplint;--filter=-legal/copyright")



include_directories(include)
file(
        GLOB
        SOURCES
        "src/*.cpp"
)

add_library(runner ${SOURCES})
target_link_options(runner PRIVATE "LINKER:-lseccomp")
target_link_libraries(runner PRIVATE -lseccomp)

add_executable(test main.cpp)
target_link_libraries(test runner)

#add_subdirectory(extern/pybind11)
#pybind11_add_module(runner_lib_py src/runner.cpp src/wrapper.cpp)