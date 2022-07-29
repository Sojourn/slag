#include "slag/error.h"
#include <cstdlib>

slag::Error::Error(ErrorCode code)
    : code_{code}
{
}

bool slag::Error::operator==(const Error& that) const {
    return code_ == that.code_;
}

bool slag::Error::operator!=(const Error& that) const {
    return !operator==(that);
}

slag::ErrorCategory slag::Error::category() const {
    return static_cast<ErrorCategory>(static_cast<uint32_t>(code_) / ERROR_CATEGORY_RANGE);
}

slag::ErrorCode slag::Error::code() const {
    return code_;
}

const char* slag::to_string(ErrorCategory error_category) {
    switch (error_category) {
#define X(SLAG_ERROR_CATEGORY) case ErrorCategory::SLAG_ERROR_CATEGORY: return #SLAG_ERROR_CATEGORY;
        SLAG_ERROR_CATEGORIES(X)
#undef X
    }

    abort();
}

const char* slag::to_string(ErrorCode error_code) {
    switch (error_code) {
#define X(SLAG_ERROR_CATEGORY, SLAG_ERROR_CODE_NAME, SLAG_ERROR_CODE_VALUE) \
        case ErrorCode::SLAG_ERROR_CODE_NAME: return #SLAG_ERROR_CATEGORY "." #SLAG_ERROR_CODE_NAME;

        SLAG_ERROR_CODES(X)
#undef X
    }

    abort();
}

const char* slag::to_string(const Error& error) {
    return to_string(error.code());
}
