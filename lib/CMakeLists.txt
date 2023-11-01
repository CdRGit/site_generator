cmake_minimum_required(VERSION 3.22)

file(GLOB_RECURSE sitegen-sources "*.c")
add_library(sitegen ${sitegen-sources})

target_include_directories(sitegen PUBLIC "${PROJECT_SOURCE_DIR}/include")
