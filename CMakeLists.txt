cmake_minimum_required (VERSION 3.12)

project (dating)

add_subdirectory (src)

file (COPY assets DESTINATION ${CMAKE_BINARY_DIR})
