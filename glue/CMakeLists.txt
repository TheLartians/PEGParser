cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

project(PEGParserGlue
  VERSION 1.0
  LANGUAGES CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Add dependencies via CPM ----

include(../cmake/CPM.cmake)

CPMFindPackage(
  NAME PEGParser
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
)

CPMFindPackage(
  NAME Glue
  GITHUB_REPOSITORY TheLartians/Glue
  VERSION 1.3.2
)

CPMAddPackage(
  NAME PackageProject.cmake
  GITHUB_REPOSITORY TheLartians/PackageProject.cmake
  VERSION 1.2
)

# ---- Add source files ----

FILE(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
FILE(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----

add_library(PEGParserGlue ${headers} ${sources})

set_target_properties(PEGParserGlue PROPERTIES CXX_STANDARD 17)
target_compile_options(PEGParserGlue PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")
target_link_libraries(PEGParserGlue PRIVATE PEGParser)
target_link_libraries(PEGParserGlue PUBLIC Glue)

target_include_directories(PEGParserGlue
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  DEPENDENCIES Glue
)
