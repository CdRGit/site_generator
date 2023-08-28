cmake_minimum_required(VERSION 3.22)

file(GLOB_RECURSE libsitegen-sources "*.c")
add_library(libsitegen ${libsitegen-sources})
target_include_directories(libsitegen PUBLIC "${PROJECT_SOURCE_DIR}/include")
