add_executable(unit_test
        catch.hpp
        unit_test.cpp
        ut_operation_state_machine.cpp
        ut_io_uring_reactor.cpp
        )

target_link_libraries(unit_test PUBLIC slag)
target_compile_features(unit_test PRIVATE cxx_std_17)
