#include "operation_table.h"
#include <cassert>

namespace slag {

    OperationTable::OperationTable(size_t initial_capacity) {
        table_.resize(initial_capacity);
        unused_table_rows_.resize(initial_capacity);

        for (RowId row_id = 0; row_id < initial_capacity; ++row_id) {
            table_[row_id] = nullptr;
            unused_table_rows_[row_id] = initial_capacity - row_id - 1;
        }
    }

    auto OperationTable::insert(Operation& operation) -> RowId {
        RowId row_id;

        // Either append to the table or reuse an empty slot.
        if (unused_table_rows_.empty()) {
            if (table_.size() == std::numeric_limits<RowId>::max()) {
                throw std::runtime_error("OperationTable is full");
            }

            row_id = static_cast<RowId>(table_.size());
            table_.push_back(&operation);
        }
        else {
            row_id = unused_table_rows_.back();
            table_[row_id] = &operation;
        }

        return row_id;
    }

    Operation& OperationTable::select(RowId row_id) {
        assert(row_id < table_.size());

        return *table_[row_id];
    }

    void OperationTable::remove(RowId row_id) {
        assert(row_id < table_.size());

        table_[row_id] = nullptr;
        unused_table_rows_.push_back(row_id);
    }

}
