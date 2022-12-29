#pragma once

#include <type_traits>
#include <variant>
#include <stdexcept>
#include "slag/void.h"
#include "slag/error.h"
#include "slag/pollable.h"

namespace slag {

    template<typename T>
    class Future;

    template<typename T>
    class Promise;

    template<typename T>
    class FutureFactory;

    template<typename T>
    using FutureValue = std::conditional_t<std::is_same_v<T, void>, Void, T>;

    template<typename T>
    using FutureError = std::exception_ptr;

    template<typename T>
    using FutureResult = std::variant<
        std::monostate,
        FutureValue<T>,
        FutureError<T>
    >;

    template<typename T>
    class Future : public Pollable {
    public:
        Future();
        Future(Future&& that);
        Future(const Future&) = delete;
        ~Future();

        Future& operator=(Future&& that);
        Future& operator=(const Future&) = delete;

        [[nodiscard]] bool has_result() const;
        [[nodiscard]] bool has_value() const;
        [[nodiscard]] bool has_error() const;

        [[nodiscard]] FutureValue<T>& get();
        [[nodiscard]] const FutureValue<T>& get() const;

        [[nodiscard]] FutureValue<T>& value();
        [[nodiscard]] const FutureValue<T>& value() const;
        [[nodiscard]] std::exception_ptr error() const;

        void reset();

    private:
        friend class FutureFactory<T>;

        explicit Future(Promise<T>& promise);

    private:
        friend class Promise<T>;

        void abandon(Promise<T>& promise);

    private:
        Promise<T>*     promise_;
        FutureResult<T> result_;
    };

    template<typename T>
    class Promise : public Pollable {
    public:
        Promise();
        Promise(Promise&& that);
        Promise(const Promise&) = delete;
        ~Promise();

        Promise& operator=(Promise&& that);
        Promise& operator=(const Promise&) = delete;

        template<typename... Args>
        void emplace_value(Args&&... args);
        void set_value(FutureValue<T> value);
        void set_default_value();
        void set_error(std::exception_ptr exception);
        void set_error(Error error, std::string_view message);

        void reset();

    private:
        friend class FutureFactory<T>;
        friend class Future<T>;

        explicit Promise(Future<T>& future);

        void abandon(Future<T>& future);

    private:
        Future<T>* future_;
    };

    // FuturePromisePair?
    template<typename T>
    struct FutureFactory {
        Future<T>  future;
        Promise<T> promise;

        FutureFactory();
    };

    template<typename T>
    [[nodiscard]] FutureFactory<T> make_future();

    template<typename T>
    [[nodiscard]] Pollable& to_pollable(Future<T>& future);

    template<typename T>
    [[nodiscard]] Pollable& to_pollable(Promise<T>& promise);

}

#include "future.hpp"
