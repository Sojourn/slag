#include "slag/operation_flags.h"
#include <cstdlib>

size_t slag::to_index(OperationFlag operation_flag) {
    return static_cast<size_t>(operation_flag);
}

const char* slag::to_string(OperationFlag operation_flag) {
    switch (operation_flag) {
#define X(SLAG_OPERATION_FLAG) case OperationFlag::SLAG_OPERATION_FLAG: return #SLAG_OPERATION_FLAG;
        SLAG_OPERATION_FLAGS(X)
#undef X
    }

    abort();
}

std::string slag::to_string(const OperationFlags& operation_flags) {
    std::string result;
    for (size_t i = 0; i < OPERATION_FLAG_COUNT; ++i) {
        if (operation_flags.test(i)) {
            if (!result.empty()) {
                result += '|';
            }

            result += to_string(static_cast<OperationFlag>(i));
        }
    }

    return result;
}
