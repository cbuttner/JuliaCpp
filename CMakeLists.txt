cmake_minimum_required(VERSION 2.8)

project(Julia++)

enable_testing()

# We need to specify the directories where julia's headers and libs might be
# found.  
if (NOT DEFINED julia-root-dir)
  message ("Using default julia-root-dir=/usr")
  include_directories("/usr/include/julia")
  link_directories("/usr/lib/x86_64-linux-gnu/julia")
  add_definitions(-DJULIA_INIT_DIR=\"/usr/lib\")
else ()
  # e.g. for building julia from git with make install,
  # mkdir build && cd build
  # cmake .. -Djulia-root-dir=/path/to/julia.git/julia-COMMIT
  include_directories( ${julia-root-dir}/include/julia )
  link_directories( ${julia-root-dir}/lib/julia )
  add_definitions(-DJULIA_INIT_DIR=\"${julia-root-dir}/lib\")
endif ()

include_directories("include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -fPIC")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_executable(
  tester 
  test/test.cpp 
  test/testArrays.cpp
  test/testGeneral.cpp
  test/testErrors.cpp)
target_link_libraries(
  tester 
  julia)

# Run tests with make test
add_test( tester tester )
