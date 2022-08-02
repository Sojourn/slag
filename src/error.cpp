#include "slag/error.h"
#include <stdexcept>
#include <type_traits>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <cstdlib>
#include <cassert>

slag::Error::Error(ErrorCode code)
    : code_{code}
{
}

bool slag::Error::operator==(Error that) const {
    return code_ == that.code_;
}

bool slag::Error::operator!=(Error that) const {
    return !operator==(that);
}

slag::ErrorCategory slag::Error::category() const {
    return categorize(code_);
}

slag::ErrorCode slag::Error::code() const {
    return code_;
}

void slag::Error::raise() const {
    throw std::runtime_error(to_string(*this)); // TODO: make a custom exception type that wraps an ErrorCode
}

slag::Error slag::make_system_error() {
    return make_system_error(errno);
}

slag::Error slag::make_system_error(int error_code) {
    assert((0 <= error_code) && (error_code < static_cast<int>(ERROR_CATEGORY_RANGE)));
    return Error{static_cast<ErrorCode>(error_code)};
}

slag::ErrorCategory slag::categorize(ErrorCode error_code) {
    return static_cast<ErrorCategory>(static_cast<uint32_t>(error_code) / ERROR_CATEGORY_RANGE);
}

std::string slag::to_string(ErrorCategory error_category) {
    switch (error_category) {
#define X(SLAG_ERROR_CATEGORY) case ErrorCategory::SLAG_ERROR_CATEGORY: return #SLAG_ERROR_CATEGORY;
        SLAG_ERROR_CATEGORIES(X)
#undef X
    }

    return fmt::format("{}", static_cast<std::underlying_type_t<ErrorCategory>>(error_category));
}

std::string slag::to_string(ErrorCode error_code) {
    switch (error_code) {
#define X(SLAG_ERROR_CATEGORY, SLAG_ERROR_CODE_NAME, SLAG_ERROR_CODE_VALUE) \
        case ErrorCode::SLAG_ERROR_CODE_NAME: return #SLAG_ERROR_CATEGORY "/" #SLAG_ERROR_CODE_NAME;

        SLAG_ERROR_CODES(X)
#undef X
    }

    return fmt::format("{}/{}", to_string(categorize(error_code)), static_cast<std::underlying_type_t<ErrorCode>>(error_code));
}

std::string slag::to_string(Error error) {
    return to_string(error.code());
}
