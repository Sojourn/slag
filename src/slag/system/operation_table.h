#pragma once

#include <stdexcept>
#include <compare>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include "slag/core.h"

namespace slag {

    class OperationTable {
    public:
        using Index = uint32_t;
        using Nonce = uint32_t;

        // NOTE: A default constructed key will never match a valid key.
        struct Key {
            Index index;
            Nonce nonce;

            auto operator<=>(const Key&) const = default;

            explicit operator bool() const {
                return *this != Key{};
            }
        };

        struct Record {
            Operation* operation = nullptr;
            Nonce nonce = 0;
        };

        explicit OperationTable(size_t initial_capacity = 1024);

        Key insert(Operation& operation);
        Operation& select(Key key);
        void remove(Key key);

    private:
        std::vector<Record>   table_;
        std::vector<uint32_t> tombstones_;
    };

    using OperationKey = OperationTable::Key;

    uint64_t encode_operation_key(OperationKey decoded_key);
    OperationKey decode_operation_key(uint64_t encoded_key);

}
