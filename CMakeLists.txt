cmake_minimum_required(VERSION 2.8)
project(Julia++)

include_directories("/usr/include/julia")
include_directories("include")

link_directories("/usr/lib/x86_64-linux-gnu/julia")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fPIC")

add_definitions(-DJULIA_INIT_DIR=\"/usr/lib\")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_executable(tester test/test.cpp test/testArrays.cpp test/testGeneral.cpp test/testErrors.cpp)
target_link_libraries(tester julia)