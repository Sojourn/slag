add_executable(unit_test
        catch.hpp
        unit_test.cpp
        ut_registry.cpp
        )

target_link_libraries(unit_test PUBLIC slag)
target_compile_features(unit_test PRIVATE cxx_std_17)
