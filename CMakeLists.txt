cmake_minimum_required(VERSION 3.13)
project(slag)

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_compile_options(-Wno-attributes) # suppress attribute ignored warnings
add_compile_options(-g) # add debug information

add_subdirectory(src)
# add_subdirectory(unit_test)
