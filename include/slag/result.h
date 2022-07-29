#pragma once

#include <type_traits>
#include <cstdint>
#include "slag/error.h"

namespace slag {

    template<typename T>
    class Result;

    template<typename T>
    class Result {
        static_assert(!std::is_same_v<T, Error>);

    public:
        explicit Result(T&& value);
        explicit Result(const T& value);
        explicit Result(Error error);
        Result(Result&& other);
        Result(const Result& other);
        ~Result();

        Result& operator=(Result&& that);
        Result& operator=(const Result& that);

        [[nodiscard]] bool has_value() const;
        [[nodiscard]] bool has_error() const;

        [[nodiscard]] T& value();
        [[nodiscard]] const T& value() const;

        [[nodiscard]] Error& error();
        [[nodiscard]] const Error& error() const;

    private:
        void cleanup();

    private:
        std::aligned_storage_t<sizeof(T), alignof(T)> value_storage_;
        Error                                         error_;
    };

    template<>
    class Result<void> {
    public:
        Result();
        explicit Result(Error error);
        Result(const Result& other);
        ~Result() = default;

        Result& operator=(const Result& that);

        [[nodiscard]] bool has_value() const;
        [[nodiscard]] bool has_error() const;

        [[nodiscard]] Error& error();
        [[nodiscard]] const Error& error() const;

    private:
        Error error_;
    };

}

#include "result.hpp"
