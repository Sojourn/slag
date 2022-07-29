#pragma once

#include <cstdint>
#include <cstddef>
#include <cerrno>

#define SLAG_ERROR_CATEGORIES(X) \
    X(SYSTEM)                    \
    X(RUNTIME)

#define SLAG_ERROR_CODES(X) \
    X(SYSTEM,  SUCCESS,          0)          \
    X(SYSTEM,  CANCELED,         ECANCELED)  \
    X(SYSTEM,  TRY_AGAIN_LATER,  EAGAIN)     \
    X(RUNTIME, PROMISE_BROKEN,   0)          \
    X(RUNTIME, FUTURE_DISCARDED, 1)

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
        explicit Error(ErrorCode code = ErrorCode::SUCCESS);

        [[nodiscard]] bool operator==(const Error& that) const;
        [[nodiscard]] bool operator!=(const Error& that) const;

        [[nodiscard]] ErrorCategory category() const;
        [[nodiscard]] ErrorCode code() const;

    private:
        ErrorCode code_;
    };

    [[nodiscard]] const char* to_string(ErrorCategory error_category);
    [[nodiscard]] const char* to_string(ErrorCode error_code);
    [[nodiscard]] const char* to_string(const Error& error);

}
