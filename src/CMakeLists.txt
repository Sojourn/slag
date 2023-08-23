set(SLAG_SOURCE_FILES
    # operation_state_machine.cpp
    # operation_types.cpp
    # operation_flags.cpp
    # operation.cpp
    # resource.cpp
    # resource_context.cpp
    # resource_context_index.cpp
    # reactor.cpp
    # io_uring_reactor.cpp
    # io_uring_resource_context.cpp
    error.cpp
    memory.cpp
    # event_loop.cpp
    # task.cpp
    platform.cpp
    file_descriptor.cpp
    address.cpp
    util.cpp
    # task_queue.cpp
    # executor.cpp
    # fiber_base.cpp
    # awaitable.cpp
    # socket.cpp
    # buffer.cpp
    # buffer_slice.cpp
    # pollable.cpp
    trace.cpp
    application.cpp
    message_fragment.cpp
    message_record_fragment.cpp
    message_buffer_fragment.cpp
    message_encoder.cpp
    message.cpp
    record_encoder.cpp
    record_decoder.cpp
    ../generated/record.cpp
    postal/buffer.cpp
    postal/buffer_ledger.cpp
    postal/buffer_reader.cpp
    postal/buffer_writer.cpp
    postal/buffer_allocator.cpp
    postal/season.cpp
    postal/domain.cpp
    postal/post_box.cpp
    postal/post_office.cpp
    postal/postal_service.cpp
    postal/task.cpp
    postal/event.cpp
    postal/selector.cpp
    postal/executor.cpp
)

set(SLAG_HEADER_FILES
    ../include/slag/slag.h
    # ../include/slag/operation_state_machine.h
    # ../include/slag/operation_parameters.h
    # ../include/slag/operation_types.h
    # ../include/slag/operation_flags.h
    # ../include/slag/operation.h
    # ../include/slag/resource.h
    # ../include/slag/resource_context.h
    # ../include/slag/resource_context_index.h
    # ../include/slag/reactor.h
    # ../include/slag/io_uring_reactor.h
    # ../include/slag/io_uring_resource_context.h
    ../include/slag/bit_set.h
    ../include/slag/bit_set.hpp
    ../include/slag/error.h
    # ../include/slag/event_loop.h
    # ../include/slag/task.h
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
    ../include/slag/visit.h
    ../include/slag/visit.hpp
    ../include/slag/task_queue.h
    # ../include/slag/fiber_base.h
    # ../include/slag/fiber.h
    # ../include/slag/fiber.hpp
    # ../include/slag/coroutine.h
    # ../include/slag/coroutine.hpp
    # ../include/slag/socket.h
    # ../include/slag/pollable.h
    # ../include/slag/selector.h
    # ../include/slag/selector.hpp
    ../include/slag/trace.h
    ../include/slag/stack.h
    ../include/slag/stack.hpp
    ../include/slag/layer.h
    ../include/slag/layer.hpp
    ../include/slag/stack_util.h
    ../include/slag/spsc_queue.h
    ../include/slag/spsc_queue.hpp
    ../include/slag/buffer_writer.h
    ../include/slag/buffer_writer.hpp
    ../include/slag/buffer_reader.h
    ../include/slag/buffer_reader.hpp
    ../include/slag/message_fragment.h
    ../include/slag/message_record_fragment.h
    ../include/slag/message_buffer_fragment.h
    ../include/slag/message_encoder.h
    ../include/slag/message.h
    ../include/slag/record_encoder.h
    ../include/slag/record_decoder.h
    ../generated/record.h
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
