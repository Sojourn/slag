#pragma once

#include <type_traits>
#include <variant>
#include <coroutine>
#include <stdexcept>
#include "slag/pollable.h"
#include "slag/fiber_base.h"
#include "slag/awaitable.h"

namespace slag {

    template<typename T>
    class Coroutine;

    struct InitialSuspend {
        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<>) const noexcept;
        void await_resume() const noexcept;
    };

    template<typename T>
    class Coroutine {
    public:
        class Promise : public Pollable {
        public:
            [[nodiscard]] Coroutine<T> get_return_object() noexcept;
            [[nodiscard]] InitialSuspend initial_suspend() const noexcept;
            [[nodiscard]] std::suspend_always final_suspend() const noexcept;
            [[nodiscard]] std::suspend_always yield_value() const;

            void return_value(T value) noexcept(std::is_nothrow_move_constructible_v<T>);
            void unhandled_exception() noexcept(std::is_nothrow_copy_constructible_v<std::exception_ptr>);

            [[nodiscard]] bool is_done() const noexcept;
            [[nodiscard]] T& value();
            [[nodiscard]] const T& value() const;

        private:
            std::variant<std::monostate, T, std::exception_ptr> result_;
        };

        using promise_type = Promise;

    private:
        explicit Coroutine(Promise& promise) noexcept;

    public:
        Coroutine() = default;
        Coroutine(Coroutine&& other) noexcept;
        Coroutine(const Coroutine&) = delete;
        ~Coroutine();

        Coroutine& operator=(Coroutine&& that) noexcept;
        Coroutine& operator=(const Coroutine&) = delete;

        [[nodiscard]] bool is_done() const noexcept;
        [[nodiscard]] T& value() noexcept;
        [[nodiscard]] const T& value() const noexcept;
        [[nodiscard]] std::coroutine_handle<> handle() noexcept;
        [[nodiscard]] Pollable& pollable() noexcept;

        void resume();

    private:
        std::coroutine_handle<Promise> handle_;
    };

    template<typename T>
    class CoroutineAwaitable : public Awaitable {
    public:
        CoroutineAwaitable(Coroutine<T> coroutine);

        [[nodiscard]] T await_resume();

    private:
        Coroutine<T> coroutine_;
    };

    template<typename T>
    [[nodiscard]] inline Pollable& to_pollable(Coroutine<T>& coroutine);

    template<typename T>
    [[nodiscard]] inline CoroutineAwaitable<T> operator co_await(Coroutine<T> coroutine);

}

#include "coroutine.hpp"
