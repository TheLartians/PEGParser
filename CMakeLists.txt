cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(SET CMP0074 NEW)

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

option(BUILD_EXAMPLES "Enable examples" OFF)
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

target_include_directories(LarsParser INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/LarsParserConfigVersion.cmake"
    VERSION v1.0
    COMPATIBILITY AnyNewerVersion
)


target_include_directories(LarsParser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LarsParser PUBLIC LHC)

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

IF(${BUILD_EXAMPLES})
  add_subdirectory(examples)    
ENDIF()

# ---- Tests ----

if(${ENABLE_TESTS})
  ENABLE_TESTING()  
  add_subdirectory(tests)  
endif()

