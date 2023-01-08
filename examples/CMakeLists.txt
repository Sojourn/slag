add_executable(example
        # scratch.cpp
        # echo_server.cpp
        fuzz_intrusive_list.cpp
        )

target_link_libraries(example PUBLIC slag)
target_compile_features(example PRIVATE cxx_std_20)
