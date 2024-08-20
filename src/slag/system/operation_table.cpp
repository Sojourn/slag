#include "operation_table.h"
#include <cassert>

namespace slag {

    OperationTable::OperationTable(const size_t initial_capacity)
        : table_(initial_capacity)
        , tombstones_(initial_capacity)
    {
        for (size_t i = 0; i < table_.size(); ++i) {
            tombstones_[i] = static_cast<Index>(table_.size() - i - 1);
        }
    }

    auto OperationTable::insert(Operation& operation) -> Key {
        Index index;
        if (tombstones_.empty()) {
            index = static_cast<Index>(table_.size());
            if (index == INVALID_INDEX) {
                throw std::runtime_error("Too many operations in progress");
            }

            table_.emplace_back();
        }
        else {
            index = tombstones_.back();
            tombstones_.pop_back();
        }

        Record& record = table_[index];
        record.operation = &operation;
        record.nonce += 1;

        return {
            .index = index,
            .nonce = record.nonce,
        };
    }

    Operation& OperationTable::select(const Key key) {
        if (table_.size() <= key.index) {
            abort();
        }

        Record& record = table_[key.index];
        if (key.nonce != record.nonce) {
            abort();
        }

        return *record.operation;
    }

    void OperationTable::remove(const Key key) {
        if (table_.size() <= key.index) {
            abort();
        }

        Record& record = table_[key.index];
        if (key.nonce != record.nonce) {
            abort();
        }

        record.operation = nullptr;
    }

    uint64_t encode_operation_key(OperationKey decoded_key) {
        uint64_t encoded_key;
        static_assert(sizeof(encoded_key) == sizeof(decoded_key));
        memcpy(&encoded_key, &decoded_key, sizeof(encoded_key));
        return encoded_key;
    }

    OperationKey decode_operation_key(uint64_t encoded_key) {
        struct {
            uint32_t index;
            uint32_t nonce;
        } primitive_decoded_key;

        static_assert(sizeof(primitive_decoded_key) == sizeof(encoded_key));
        memcpy(&primitive_decoded_key, &encoded_key, sizeof(primitive_decoded_key));

        OperationKey decoded_key;
        decoded_key.index = primitive_decoded_key.index;
        decoded_key.nonce = primitive_decoded_key.nonce;
        return decoded_key;
    }

}
