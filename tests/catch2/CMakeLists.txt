cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.7.0
  UPDATE_DISCONNECTED 1
  QUIET
)

FetchContent_GetProperties(catch2)
if(NOT catch2_POPULATED)
  FetchContent_Populate(catch2)
endif()

add_library(Catch2 INTERFACE)

target_include_directories(Catch2
  INTERFACE
    $<BUILD_INTERFACE:${catch2_SOURCE_DIR}/single_include>
)
