cmake_minimum_required(VERSION 3.22)

project(sitegen
    VERSION 0.1.0
    LANGUAGES C
)

set(C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wpedantic -Wall -Werror -Wshadow -Wconversion -Wno-unused -Wno-infinite-recursion -fsanitize=address -g)
add_link_options(-fsanitize=address)

add_subdirectory(lib)

add_executable(sitegen_driver "${PROJECT_SOURCE_DIR}/src/driver.c")
set_property(TARGET sitegen_driver PROPERTY C_STANDARD 17)
set_property(TARGET sitegen_driver PROPERTY C_STANDARD_REQUIRED true)
target_link_libraries(sitegen_driver sitegen)
target_include_directories(sitegen_driver PUBLIC "${PROJECT_SOURCE_DIR}/include")
