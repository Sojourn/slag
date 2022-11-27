set(SLAG_SOURCE_FILES
    operation_state_machine.cpp
    operation_types.cpp
    operation_flags.cpp
    operation.cpp
    resource.cpp
    resource_context.cpp
    resource_context_index.cpp
    reactor.cpp
    io_uring_reactor.cpp
    io_uring_resource_context.cpp
    error.cpp
    event.cpp
    event_loop.cpp
    task.cpp
    platform.cpp
    file_descriptor.cpp
    address.cpp
    util.cpp
    task_queue.cpp
    executor.cpp
    fiber_base.cpp
    awaitable.cpp
    awaitable_base.cpp
    socket.cpp
    buffer.cpp
    buffer_slice.cpp
    byte_stream.cpp
    pollable.cpp
    trace.cpp
)

set(SLAG_HEADER_FILES
    ../include/slag/slag.h
    ../include/slag/operation_state_machine.h
    ../include/slag/operation_parameters.h
    ../include/slag/operation_types.h
    ../include/slag/operation_flags.h
    ../include/slag/operation.h
    ../include/slag/resource.h
    ../include/slag/resource_context.h
    ../include/slag/resource_context_index.h
    ../include/slag/reactor.h
    ../include/slag/io_uring_reactor.h
    ../include/slag/io_uring_resource_context.h
    ../include/slag/error.h
    ../include/slag/event.h
    ../include/slag/event_loop.h
    ../include/slag/task.h
    ../include/slag/platform.h
    ../include/slag/file_descriptor.h
    ../include/slag/address.h
    ../include/slag/result.h
    ../include/slag/result.hpp
    ../include/slag/tagged_pointer.h
    ../include/slag/tagged_pointer.hpp
    ../include/slag/pool_allocator.h
    ../include/slag/pool_allocator.hpp
    ../include/slag/logging.h
    ../include/slag/util.h
    ../include/slag/task_queue.h
    ../include/slag/fiber_base.h
    ../include/slag/fiber.h
    ../include/slag/fiber.hpp
    ../include/slag/coroutine.h
    ../include/slag/coroutine.hpp
    ../include/slag/socket.h
    ../include/slag/byte_stream.h
    ../include/slag/pollable.h
    ../include/slag/selector.h
    ../include/slag/selector.hpp
    ../include/slag/trace.h
)

add_library(slag ${SLAG_SOURCE_FILES} ${SLAG_HEADER_FILES})
target_link_libraries(slag uring)
target_link_libraries(slag fmt)
target_include_directories(slag
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PRIVATE
            $<CMAKE_CURRENT_SOURCE_DIR}
        )

set_target_properties(slag PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )
