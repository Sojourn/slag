add_subdirectory(slag)

add_executable(test main.cpp)
target_link_libraries(test PUBLIC slag)
target_compile_features(test PRIVATE cxx_std_20)

