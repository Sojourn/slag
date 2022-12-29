#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include "slag/future.h"
#include "slag/platform.h"
#include "slag/operation_types.h"
#include "slag/file_descriptor.h"
#include "slag/address.h"
#include "slag/handle.h"
#include "slag/buffer.h"
#include "slag/buffer_slice.h"

namespace slag {

    class Operation;

    template<OperationType operation_type>
    struct OperationParameters;

    template<>
    struct OperationParameters<OperationType::NOP> {
    };

    template<>
    struct OperationParameters<OperationType::CANCEL> {
        // TODO: verify that these operations have a nurse-patient relationship
        Operation* target_operation; // the operation we are trying to cancel
    };

    template<>
    struct OperationParameters<OperationType::ASSIGN> {
        struct {
            FileDescriptor file_descriptor;
        } arguments;

        FutureFactory<void> result;
    };

    template<>
    struct OperationParameters<OperationType::CLOSE> {
    };

    template<>
    struct OperationParameters<OperationType::BIND> {
        struct {
            Address address;
        } arguments;

        FutureFactory<void> result;
    };

    template<>
    struct OperationParameters<OperationType::LISTEN> {
        struct {
            int backlog = 0;
        } arguments;

        FutureFactory<void> result;
    };

    template<>
    struct OperationParameters<OperationType::CONNECT> {
        Address address;
    };

    template<>
    struct OperationParameters<OperationType::ACCEPT> {
        FutureFactory<std::pair<FileDescriptor, Address>> result;

        Address   address;
        socklen_t address_length = 0;
    };

    template<>
    struct OperationParameters<OperationType::SEND> {
        struct {
            BufferSlice buffer_slice;
        } arguments;

        FutureFactory<size_t> result;
    };

    template<>
    struct OperationParameters<OperationType::RECEIVE> {
        struct {
            size_t count = 0;
        } arguments;

        Handle<Buffer>             buffer;
        FutureFactory<BufferSlice> result;
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
