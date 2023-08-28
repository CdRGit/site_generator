cmake_minimum_required(VERSION 3.10)

project(sitegen)

set(SOURCE_FILES src/lib/site_generator.c)
add_library(sitegen ${SOURCE_FILES})
