#pragma once

#include <type_traits>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include "slag/error.h"

namespace slag {

    template<typename T>
    class Result;

    template<typename T>
    class Result {
        static_assert(!std::is_same_v<T, Error>, "Result cannot hold an error as a value");

    public:
        explicit Result(T&& value);
        explicit Result(const T& value);
        Result(Error error);
        Result(Result&& other);
        Result(const Result& other);
        ~Result();

        Result& operator=(Result&& that);
        Result& operator=(const Result& that);

        [[nodiscard]] explicit operator bool() const {
            return has_value();
        }

        [[nodiscard]] bool has_value() const;
        [[nodiscard]] bool has_error() const;

        [[nodiscard]] T& value();
        [[nodiscard]] const T& value() const;

        [[nodiscard]] Error& error();
        [[nodiscard]] const Error& error() const;

    private:
        void cleanup();

    private:
        alignas(alignof(T)) std::byte value_storage_[sizeof(T)];
        Error                         error_;
    };

    template<>
    class Result<void> {
    public:
        Result()
            : error_{ErrorCode::SUCCESS}
        {
        }

        Result(Error error)
            : error_{error}
        {
            if (!has_error()) {
                abort();
            }
        }

        Result(const Result&) = default;

        Result& operator=(const Result&) = default;

        [[nodiscard]] explicit operator bool() const {
            return has_value();
        }

        [[nodiscard]] bool has_value() const {
            return error_.code() == ErrorCode::SUCCESS;
        }

        [[nodiscard]] bool has_error() const {
            return !has_value();
        }

        [[nodiscard]] Error& error() {
            assert(has_error());
            return error_;
        }

        [[nodiscard]] const Error& error() const {
            assert(has_error());
            return error_;
        }

    private:
        Error error_;
    };

}

#include "result.hpp"
