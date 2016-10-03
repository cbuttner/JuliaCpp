cmake_minimum_required(VERSION 2.8)

project(Julia++)

enable_testing()

set(JULIA_ROOT_DIR "/usr" CACHE PATH "Julia root directory.")
set(JULIA_COMMAND "${JULIA_ROOT_DIR}/bin/julia")

if(NOT EXISTS ${JULIA_COMMAND})
  message(FATAL_ERROR "${JULIA_ROOT_DIR}/bin/julia does not exist. Please set JULIA_ROOT_DIR to the Julia root directory containing bin/julia.")
endif()

execute_process(COMMAND ${JULIA_COMMAND} "-e" "print(joinpath(JULIA_HOME,Base.DATAROOTDIR,\"julia\"))" OUTPUT_VARIABLE JL_SHARE)
execute_process(COMMAND ${JULIA_COMMAND} "${JL_SHARE}/julia-config.jl" "--cflags" OUTPUT_VARIABLE JL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${JULIA_COMMAND} "${JL_SHARE}/julia-config.jl" "--ldflags" OUTPUT_VARIABLE JL_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${JULIA_COMMAND} "${JL_SHARE}/julia-config.jl" "--ldlibs" OUTPUT_VARIABLE JL_LDLIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
message("Julia share dir: ${JL_SHARE}")
message("Julia cflags: ${JL_CFLAGS}")
message("Julia ldflags: ${JL_LDFLAGS}")
message("Julia ldlibs: ${JL_LDLIBS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror ${JL_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${JL_LDFLAGS} ${JL_LDLIBS}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

include_directories("include")

add_executable(
  tester
  test/test.cpp
  test/testArrays.cpp
  test/testGeneral.cpp
  test/testErrors.cpp)
target_link_libraries(tester)

# Run tests with make test
add_test(tester tester)
