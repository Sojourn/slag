#pragma once

#include <algorithm>
#include "slag/operation_types.h"
#include "slag/file_descriptor.h"

namespace slag {

    template<OperationType operation_type>
    struct OperationParameters;

    template<>
    struct OperationParameters<OperationType::NOP> {
    };

    template<>
    struct OperationParameters<OperationType::ASSIGN_FILE_DESCRIPTOR> {
        FileDescriptor file_descriptor;
    };

    template<>
    struct OperationParameters<OperationType::CLOSE_FILE_DESCRIPTOR> {
    };

    constexpr size_t max_operation_parameters_size() {
        size_t size = 0;
#define X(SLAG_OPERATION_TYPE) size = std::max(sizeof(OperationParameters<OperationType::SLAG_OPERATION_TYPE>), size);
        SLAG_OPERATION_TYPES(X)
#undef X
        return size;
    }

    constexpr size_t max_operation_parameters_alignment() {
        size_t alignment = 0;
#define X(SLAG_OPERATION_TYPE) alignment = std::max(alignof(OperationParameters<OperationType::SLAG_OPERATION_TYPE>), alignment);
        SLAG_OPERATION_TYPES(X)
#undef X
        return alignment;
    }

}
