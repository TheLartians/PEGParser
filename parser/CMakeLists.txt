
file (GLOB headers "*.h")

add_library(LarsParser EXCLUDE_FROM_ALL ${headers} )
SET_TARGET_PROPERTIES(LarsParser PROPERTIES LINKER_LANGUAGE CXX)

