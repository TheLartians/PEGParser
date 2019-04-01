include(ExternalProject)
find_package(Wget REQUIRED)

ExternalProject_Add(lhc-project
  GIT_REPOSITORY https://github.com/TheLartians/LHC.git
  GIT_TAG        master
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(lhc-project download_dir)

add_library(LHC INTERFACE)
target_include_directories(LHC INTERFACE ${download_dir}/lhc-project/include)
SET(LHC_INCLUDE_DIRS ${download_dir}/lhc-project/include)
