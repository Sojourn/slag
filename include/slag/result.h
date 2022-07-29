#pragma once

#include <type_traits>
#include <cstdint>
#include "slag/error.h"

namespace slag {

    template<typename T>
    class Result {
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
        enum class Contents : uint8_t {
            VALUE,
            ERROR,
        };

        std::aligned_storage_t<
            std::max(sizeof(T), sizeof(Error)),
            std::max(alignof(T), alignof(Error))
        > storage_;

        Contents contents_;
    };

}

#include "result.hpp"
