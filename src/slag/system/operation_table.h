#pragma once

#include "slag/object.h"
#include "slag/resource.h"

#include <stdexcept>
#include <compare>
#include <vector>
#include <limits>
#include <cstdlib>
#include <cstdint>
#include <cstddef>

namespace slag {

    class OperationTable {
    public:
        using Index = uint32_t;
        using Nonce = uint32_t;

        static constexpr Index INVALID_INDEX = std::numeric_limits<Index>::max();
        static constexpr Nonce INVALID_NONCE = std::numeric_limits<Nonce>::max();

        struct Key {
            Index index = INVALID_INDEX;
            Nonce nonce = INVALID_NONCE;

            explicit operator bool() const {
                return *this != Key{};
            }

            auto operator<=>(const Key&) const = default;
        };

        struct Record {
            Operation* operation = nullptr;
            Nonce nonce = 0;
        };

    public:
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
