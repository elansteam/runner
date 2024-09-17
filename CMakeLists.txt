cmake_minimum_required(VERSION 3.25)
project(cpp-elans-runner)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_CPPLINT "cpplint;--filter=-legal/copyright")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lseccomp")


include_directories(include)
file(
        GLOB
        SOURCES
        "src/*.cpp"
)

add_library(runner ${SOURCES})

add_executable(test main.cpp)
target_link_libraries(test runner)

#add_subdirectory(extern/pybind11)
#pybind11_add_module(runner_lib_py src/runner.cpp src/wrapper.cpp)