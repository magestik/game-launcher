add_subdirectory(rlottie)
target_include_directories(rlottie INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rlottie/inc>)
