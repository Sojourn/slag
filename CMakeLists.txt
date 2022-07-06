cmake_minimum_required(VERSION 3.13)
project(slag)

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_compile_options(-Wno-attributes)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(unit_test)
