add_executable(unit_test
        catch.hpp
        unit_test.cpp
        ut_result.cpp
        ut_future.cpp
        ut_task_queue.cpp
        ut_intrusive_list.cpp
        ut_executor.cpp
        ut_operation_state_machine.cpp
        ut_io_uring_reactor.cpp
        ut_lru16.cpp
        ut_transform.cpp
        ut_message.cpp
        ut_record.cpp
        ut_spsc_queue.cpp
        )

target_link_libraries(unit_test PUBLIC slag)
target_compile_features(unit_test PRIVATE cxx_std_20)
