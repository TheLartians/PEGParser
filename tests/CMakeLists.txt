cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- Dependencies ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  find_package(LarsParser REQUIRED)
endif()

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.5.0
)

FetchContent_MakeAvailable(Catch2)

# ---- Create binary ----

file(GLOB tests_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(lars-parser-tests ${tests_sources})
target_link_libraries(lars-parser-tests PUBLIC LarsParser Catch2)
set_target_properties(lars-parser-tests PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-Wall -pedantic -Wextra -Werror")

# ---- Add tests ----

ENABLE_TESTING() 
ADD_TEST(lars-parser-tests lars-parser-tests)

# ---- code coverage ----

if (${ENABLE_TEST_COVERAGE})
  set_target_properties(lars-parser-tests PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage --coverage")
  target_link_options(lars-parser-tests PUBLIC "--coverage")
endif()
