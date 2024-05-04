cmake_minimum_required(VERSION 3.22)
project(cpp-elans-runner)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lseccomp")

include_directories(include)
add_library(runner src/runner.cpp
        include/runner/cgroup_manager.h
        include/runner/domain.h
        include/runner/mount.h
        src/mount.cpp
        src/cgroup_manager.cpp)

add_executable(test main.cpp)
target_link_libraries(test runner)

#add_subdirectory(extern/pybind11)
#pybind11_add_module(runner_lib_py src/runner.cpp src/wrapper.cpp)