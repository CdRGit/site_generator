cmake_minimum_required(VERSION 3.22)

project(sitegen
    VERSION 0.1.0
    LANGUAGES C
)

set(C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Werror=return-type -Wshadow -Wconversion -Wno-unused -fsanitize=address -g)
add_link_options(-fsanitize=address)

add_subdirectory(lib)

add_executable(sitegen "${PROJECT_SOURCE_DIR}/src/driver.c")
set_property(TARGET sitegen PROPERTY C_STANDARD 17)
set_property(TARGET sitegen PROPERTY C_STANDARD_REQUIRED true)
target_link_libraries(sitegen libsitegen)
target_include_directories(sitegen PUBLIC "${PROJECT_SOURCE_DIR}/include")
