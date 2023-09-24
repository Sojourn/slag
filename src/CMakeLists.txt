set(SLAG_SOURCE_FILES
    error.cpp
    memory.cpp
    address.cpp
    util.cpp
    trace.cpp
    token_bucket.cpp
    message_fragment.cpp
    message_record_fragment.cpp
    message_buffer_fragment.cpp
    message_encoder.cpp
    message.cpp
    record_encoder.cpp
    record_decoder.cpp
    ../generated/record.cpp
    core/season.cpp
    core/domain.cpp
    core/task.cpp
    core/event.cpp
    core/selector.cpp
    core/executor.cpp
    core/task/buffered_writer_task.cpp
    system/reactor.cpp
    postal/buffer.cpp
    postal/buffer_ledger.cpp
    postal/buffer_reader.cpp
    postal/buffer_writer.cpp
    postal/buffer_allocator.cpp
    postal/cohort_buffer_allocator.cpp
    postal/post_box.cpp
    postal/post_office.cpp
    postal/postal_service.cpp
    postal/operation_base.cpp
    postal/file_handle.cpp
    postal/file_table.cpp
)

set(SLAG_HEADER_FILES
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
