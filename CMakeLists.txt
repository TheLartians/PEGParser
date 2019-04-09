cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# ---- Project ----

project(LarsParser 
  VERSION 1.4
  LANGUAGES CXX
)

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Configuration variables ----

option(BUILD_LARS_PARSER_EXAMPLES "Enable examples" OFF)
option(ENABLE_LARS_PARSER_TESTS "Enable tests" OFF)

# ---- Dependencies ----

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

CPMAddPackage(
  NAME LHC
  GIT_REPOSITORY https://github.com/TheLartians/LHC.git
  VERSION 0.3
)

# ---- Minimum CXX ----

if(NOT CMAKE_CXX_STANDARD GREATER 17)
  set(CMAKE_CXX_STANDARD 17)
endif()

# ---- Create library ----

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/*.h")
FILE(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
add_library(LarsParser ${sources} ${headers})

target_include_directories(LarsParser 
  INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/LarsParserConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

target_link_libraries(LarsParser PRIVATE LHC)
target_include_directories(LarsParser
  PUBLIC
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
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

IF(${BUILD_LARS_PARSER_EXAMPLES})
  add_subdirectory(examples)    
ENDIF()

# ---- Tests ----

if(${ENABLE_LARS_PARSER_TESTS})
  ENABLE_TESTING()  
  add_subdirectory(tests)  
endif()

