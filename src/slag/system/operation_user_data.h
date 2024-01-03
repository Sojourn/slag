#pragma once

#include <cstring>
#include <cstdint>
#include "operation_types.h"

namespace slag {

    using OperationSlot = uint8_t;

    struct OperationUserData {
        uint32_t      index;
        OperationType type;
        OperationSlot slot;

        struct {
            bool multishot;
            bool interrupt;
        } flags;
    };
    static_assert(sizeof(OperationUserData) == sizeof(uint64_t));

    inline uint64_t encode(OperationUserData user_data) {
        uint64_t encoded_user_data;
        memcpy(&encoded_user_data, &user_data, sizeof(encoded_user_data));
        return encoded_user_data;
    }

    inline OperationUserData decode(uint64_t encoded_user_data) {
        OperationUserData user_data;
        memcpy(&user_data, &encoded_user_data, sizeof(user_data));
        return user_data;
    }

}
