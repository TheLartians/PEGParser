cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

project(LarsParser 
  VERSION 1.9
  LANGUAGES CXX
)

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Configuration variables ----

option(LARS_PARSER_BUILD_EXAMPLES "Enable examples" OFF)
option(LARS_PARSER_ENABLE_TESTS "Enable tests" OFF)
option(LARS_PARSER_BUILD_GLUE_ELEMENT "Build LarsParser Glue extension" OFF)

# ---- Dependencies ----

if(${LARS_PARSER_ENABLE_TESTS})
  set(LARS_PARSER_BUILD_GLUE_ELEMENT ON)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

CPMAddPackage(
  NAME LHC
  GIT_REPOSITORY https://github.com/TheLartians/LHC.git
  VERSION 0.8
)

if(${LARS_PARSER_BUILD_GLUE_ELEMENT})
  CPMAddPackage(
    NAME Glue
    GIT_REPOSITORY https://github.com/TheLartians/Glue.git
    VERSION 0.9.5-n
    GIT_TAG cc1ad543ac09988c67f8f9eec069a0f208b8f5a6
  )
endif()

# ---- Create library ----

SET(headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/parser/grammar.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/parser/parser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/parser/interpreter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/parser/generator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/parser/peg.h"
)

SET(sources 
  "${CMAKE_CURRENT_SOURCE_DIR}/source/grammar.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/interpreter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/peg.cpp"
)

if(${LARS_PARSER_BUILD_GLUE_ELEMENT})
  LIST(APPEND headers "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/parser/glue.h")
  LIST(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/source/glue.cpp")
endif()

add_library(LarsParser ${sources} ${headers})
set_target_properties(LarsParser PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-Wall -pedantic -Wextra")

target_link_libraries(LarsParser PRIVATE LHC)

if(${LARS_PARSER_BUILD_GLUE_ELEMENT})
  target_link_libraries(LarsParser PUBLIC Glue)
endif()

target_include_directories(LarsParser
  PUBLIC
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/LarsParserConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# ---- Install ----

install(
  TARGETS LarsParser
  EXPORT LarsParserTargets
  LIBRARY DESTINATION lib COMPONENT Runtime
  ARCHIVE DESTINATION lib COMPONENT Development
  RUNTIME DESTINATION bin COMPONENT Runtime
  PUBLIC_HEADER DESTINATION include COMPONENT Development
  BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/LarsParserConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/LarsParserConfig.cmake"
  INSTALL_DESTINATION lib/cmake/LarsParser
)

install(
  EXPORT LarsParserTargets 
  DESTINATION lib/cmake/LarsParser
)

install(
  FILES 
    "${PROJECT_BINARY_DIR}/LarsParserConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/LarsParserConfig.cmake"
  DESTINATION 
    lib/cmake/LarsParser
)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION include
)

# ---- Examples ----

IF(${LARS_PARSER_BUILD_EXAMPLES})
  add_subdirectory(examples)    
ENDIF()

# ---- Tests ----

if(${LARS_PARSER_ENABLE_TESTS})
  ENABLE_TESTING()  
  add_subdirectory(tests)  
endif()

