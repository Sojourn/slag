#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "operation.h"

namespace slag {

    class OperationTable {
    public:
        using RowId = uint32_t;

        explicit OperationTable(size_t initial_capacity = 16 * 1024);

        RowId insert(Operation& operation);
        Operation& select(RowId row_id);
        void remove(RowId row_id);

    private:
        std::vector<Operation*> table_;
        std::vector<uint32_t>   unused_table_rows_;
    };

}
