add_executable(example
        poll.cpp
        )

target_link_libraries(example PUBLIC slag)
target_compile_features(example PRIVATE cxx_std_20)
