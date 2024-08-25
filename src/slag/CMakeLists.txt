set(SLAG_SOURCE_FILES
    runtime.cpp
    thread.cpp
    event_loop.cpp
    thread_context.cpp
    core/task.cpp
    core/event.cpp
    core/selector.cpp
    core/executor.cpp
    memory/buffer.cpp
    system/reactor.cpp
    system/operation_table.cpp
)

set(SLAG_HEADER_FILES
    system/operation_table.h
    system/primitive_operation.h
    system/operations/nop_operation.h
)

add_library(slag ${SLAG_SOURCE_FILES} ${SLAG_HEADER_FILES})
target_link_libraries(slag mantle)
target_link_libraries(slag uring)
target_link_libraries(slag fmt)
target_include_directories(slag
        PUBLIC
            # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
            ${PROJECT_SOURCE_DIR}/src
        PRIVATE
            # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        )

set_target_properties(slag PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

