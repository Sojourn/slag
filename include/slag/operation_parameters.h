#pragma once

#include <algorithm>
#include "slag/operation_types.h"
#include "slag/file_descriptor.h"
#include "slag/address.h"

namespace slag {

    class Operation;

    template<OperationType operation_type>
    struct OperationParameters;

    template<>
    struct OperationParameters<OperationType::NOP> {
    };

    template<>
    struct OperationParameters<OperationType::CANCEL> {
        Operation* target_operation; // the operation we are trying to cancel
    };

    template<>
    struct OperationParameters<OperationType::ASSIGN> {
        FileDescriptor file_descriptor;
    };

    template<>
    struct OperationParameters<OperationType::CLOSE> {
    };

    template<>
    struct OperationParameters<OperationType::CONNECT> {
        Address address;
    };

    template<>
    struct OperationParameters<OperationType::ACCEPT> {
        FileDescriptor file_descriptor; // out
    };

    template<>
    struct OperationParameters<OperationType::SEND> {
    };

    template<>
    struct OperationParameters<OperationType::RECEIVE> {
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
