include(ExternalProject)
find_package(Wget REQUIRED)

# Old code for use as git ExternalProject_Add example:
#ExternalProject_Add(catch2-project
#  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#  GIT_TAG        v2.7.0
#)

ExternalProject_Add(
  catch2-project
  PREFIX ${CMAKE_BINARY_DIR}/catch2
  DOWNLOAD_DIR catch2
  DOWNLOAD_COMMAND ${WGET_EXECUTABLE} https://github.com/catchorg/Catch2/releases/download/v2.7.0/catch.hpp
  TIMEOUT 10
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(catch2-project download_dir)

add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE ${download_dir}/..)
add_library(catch2::catch ALIAS catch2)
