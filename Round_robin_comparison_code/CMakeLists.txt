cmake_minimum_required(VERSION 3.0.0)
project(ao-cpp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")
add_executable(fj-test test/main.cpp src/scheduler.cpp src/active_object.cpp src/future.cpp)

target_include_directories(fj-test PUBLIC include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
