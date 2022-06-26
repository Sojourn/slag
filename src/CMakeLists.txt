set(SLAG_SOURCE_FILES
    operation_state_machine.cpp
    operation_parameters.cpp
    operation_types.cpp
    operation.cpp
    resource.cpp
    resource_context.cpp
    driver.cpp
    event_loop.cpp
    file_descriptor.cpp
    util.cpp
)

set(SLAG_HEADER_FILES
    ../include/slag/slag.h
    ../include/slag/operation_state_machine.h
    ../include/slag/operation_parameters.cpp
    ../include/slag/operation_types.cpp
    ../include/slag/operation.h
    ../include/slag/resource.h
    ../include/slag/resource_context.h
    ../include/slag/driver.h
    ../include/slag/event_loop.h
    ../include/slag/file_descriptor.h
    ../include/slag/util.h
)

add_library(slag ${SLAG_SOURCE_FILES} ${SLAG_HEADER_FILES})
target_link_libraries(slag uring)
target_include_directories(slag
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PRIVATE
            $<CMAKE_CURRENT_SOURCE_DIR}
        )

set_target_properties(slag PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )
