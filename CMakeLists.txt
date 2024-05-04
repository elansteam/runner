cmake_minimum_required(VERSION 3.22)
project(cpp-elans-runner)

set(CMAKE_CXX_STANDARD 23)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lseccomp")
#set(CMAKE_LINK_LIBRARY_FLAG "${CMAKE_LINK_LIBRARY_FLAG} -lseccomp")
find_package(PkgConfig REQUIRED)
pkg_check_modules(SECCOMP REQUIRED libseccomp)
include_directories(${SECCOMP_INCLUDE_DIRS})

include_directories(include)
add_library(${PROJECT_NAME} src/runner.cpp
        include/runner/cgroup_manager.h
        include/runner/domain.h
        include/runner/mount_manager.h
        src/mount_manager.cpp
        src/cgroup_manager.cpp
        include/runner/seccomp_manager.h)

add_executable(test main.cpp)
target_link_libraries(test ${PROJECT_NAME})
target_link_libraries(test ${SECCOMP_LIBRARIES})
target_compile_options(test PUBLIC ${SECCOMP_CFLAGS_OTHER})

#add_subdirectory(extern/pybind11)
#pybind11_add_module(runner_lib_py src/runner.cpp src/wrapper.cpp)