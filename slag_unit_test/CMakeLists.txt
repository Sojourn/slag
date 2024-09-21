add_executable(slag_unit_test
        catch.hpp
        slag_unit_test.cpp
        ut_topology.cpp
        )

target_link_libraries(slag_unit_test PUBLIC slag)
target_compile_features(slag_unit_test PRIVATE cxx_std_20)
