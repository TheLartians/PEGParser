cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
# ---- Hunter ----
include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.150.tar.gz"
    SHA1 "7f1604b4dd3fb3e9478fa5a60928486df3a6b5cb"
    LOCAL # <----- load cmake/Hunter/config.cmake
)

# ---- Project ----
project(LarsParser CXX)

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Configuration variables ----
option(ENABLE_EXAMPLES "Enable examples" OFF)
option(ENABLE_TESTS "Enable tests" OFF)

# ---- Dependencies ----
hunter_add_package(LHC)
find_package(LHC REQUIRED)

# ---- Minimum CXX ----
if(NOT CMAKE_CXX_STANDARD GREATER 17)
  set(CMAKE_CXX_STANDARD 17)
endif()

# ---- Create library ----
FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/*.h")
FILE(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
add_library(LarsParser ${sources} ${headers})

target_include_directories(LarsParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LarsParser LHC)

# ---- Export ----
set(LARS_PARSER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/" ${LHC_INCLUDE_DIRS} CACHE INTERNAL "The Lars::Parser include dirs")
set(LARS_PARSER_LIBRARIES LarsParser CACHE INTERNAL "The Lars::Parser libraries")

# ---- Examples ----
IF(${ENABLE_EXAMPLES})
  file( GLOB example_sources ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp )
  foreach( example_source_file ${example_sources} )
    get_filename_component(filename ${example_source_file} NAME)
    string(REPLACE ".cpp" "" example_name ${filename} )
    add_executable(${example_name} ${example_source_file})
    include_directories(${LARS_PARSER_INCLUDE_DIRS})
    target_link_libraries(${example_name} ${LARS_PARSER_LIBRARIES})            
  endforeach()
ENDIF()

# ---- Tests ----
IF(${ENABLE_TESTS})
  ENABLE_TESTING()  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_catch2.cmake)
  file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
  add_executable(lars-parser-tests ${test_sources})
  target_include_directories(lars-parser-tests PRIVATE ${LARS_PARSER_INCLUDE_DIRS})
  add_dependencies(lars-parser-tests catch2-project LarsParser)
  target_link_libraries(lars-parser-tests catch2 ${LARS_PARSER_LIBRARIES})        
  ADD_TEST(lars-parser lars-parser-tests)  
ENDIF()

