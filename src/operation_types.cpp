#include "slag/operation_types.h"
#include <cstdlib>

size_t slag::to_index(OperationType operation_type) {
    return static_cast<size_t>(operation_type);
}

const char* slag::to_string(OperationType operation_type) {
    switch (operation_type) {
#define X(SLAG_OPERATION_TYPE) case OperationType::SLAG_OPERATION_TYPE: return #SLAG_OPERATION_TYPE;
        SLAG_OPERATION_TYPES(X)
#undef X
    }

    abort();
}
