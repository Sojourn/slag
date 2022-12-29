#pragma once

#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <cerrno>

#define SLAG_ERROR_CATEGORIES(X) \
    X(SYSTEM)                    \
    X(RUNTIME)

// TODO: define more system error codes
#define SLAG_ERROR_CODES(X) \
    X(SYSTEM,  SUCCESS,                   0)          \
    X(SYSTEM,  CANCELED,                  ECANCELED)  \
    X(SYSTEM,  TRY_AGAIN_LATER,           EAGAIN)     \
    X(SYSTEM,  INVALID_ARGUMENT,          EINVAL)     \
    X(RUNTIME, PROMISE_BROKEN,            0)          \
    X(RUNTIME, FUTURE_ALREADY_RETRIEVED,  1)          \
    X(RUNTIME, PROMISE_ALREADY_SATISFIED, 2)          \
    X(RUNTIME, FUTURE_NOT_READY,          3)          \
    X(RUNTIME, FUTURE_DETACHED,           4)

namespace slag {

    static constexpr uint32_t ERROR_CATEGORY_RANGE = 1 << 16;

    enum class ErrorCategory : uint32_t {
#define X(SLAG_ERROR_CATEGORY) SLAG_ERROR_CATEGORY,
        SLAG_ERROR_CATEGORIES(X)
#undef X
    };

    enum class ErrorCode : uint32_t {
#define X(SLAG_ERROR_CATEGORY, SLAG_ERROR_CODE_NAME, SLAG_ERROR_CODE_VALUE) \
        SLAG_ERROR_CODE_NAME = SLAG_ERROR_CODE_VALUE + (static_cast<uint32_t>(ErrorCategory::SLAG_ERROR_CATEGORY) * ERROR_CATEGORY_RANGE),

        SLAG_ERROR_CODES(X)
#undef X
    };

    class Error {
    public:
        Error(ErrorCode code);

        [[nodiscard]] bool operator==(Error that) const;
        [[nodiscard]] bool operator!=(Error that) const;

        [[nodiscard]] ErrorCategory category() const;
        [[nodiscard]] ErrorCode code() const;

        void raise(std::string_view message) const;

        [[nodiscard]] explicit operator bool() const {
            return code_ != ErrorCode::SUCCESS;
        }

    private:
        ErrorCode code_;
    };

    [[nodiscard]] Error make_system_error();
    [[nodiscard]] Error make_system_error(int error_code);

    [[nodiscard]] ErrorCategory categorize(ErrorCode error_code);

    [[nodiscard]] std::string to_string(ErrorCategory error_category);
    [[nodiscard]] std::string to_string(ErrorCode error_code);
    [[nodiscard]] std::string to_string(Error error);

    [[nodiscard]] std::exception_ptr to_exception(Error error, std::string_view message);

}
