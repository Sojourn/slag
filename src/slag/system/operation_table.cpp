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
            if ((table_.size() + 1) == std::numeric_limits<Index>::max()) {
                throw std::runtime_error("Too many operations in progress");
            }

            index = static_cast<Index>(table_.size());
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
        if (key.index < table_.size()) {
            abort();
        }

        Record& record = table_[key.index];
        if (key.nonce != record.nonce) {
            abort();
        }

        return *record.operation;
    }

    void OperationTable::remove(const Key key) {
        if (key.index < table_.size()) {
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
        OperationKey decoded_key;
        static_assert(sizeof(decoded_key) == sizeof(encoded_key));
        memcpy(&decoded_key, &encoded_key, sizeof(decoded_key));
        return decoded_key;
    }

}
