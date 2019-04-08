cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if(TARGET LHC)
  return()
endif()

set(lhc_min_version 0.3)

find_package(LHC ${lhc_min_version} QUIET)

if(${LHC_FOUND})
  set_target_properties(LHC 
    PROPERTIES
      IMPORTED_GLOBAL True
  )
else()
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DownloadProject.cmake)

  download_project(
    PROJ LHC
    GIT_REPOSITORY https://github.com/TheLartians/LHC.git
    GIT_TAG "v${lhc_min_version}"
    UPDATE_DISCONNECTED 1
    QUIET
  )

  add_subdirectory(${LHC_SOURCE_DIR} ${LHC_BINARY_DIR})
endif()

