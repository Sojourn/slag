add_executable(example
        network_stack.cpp
        )

target_link_libraries(example PUBLIC slag)
target_compile_features(example PRIVATE cxx_std_20)
