#pragma once

#include <variant>
#include <coroutine>
#include <cassert>
#include "slag/task.h"
#include "slag/event.h"
#include "slag/future.h"
#include "slag/coroutine.h"
#include "slag/fiber_base.h"
#include "slag/awaitable_base.h"

namespace slag {

    // TODO: swap the names of Task and Fiber
    template<typename T>
    class Fiber : public FiberBase {
    public:
        template<typename CoroutineFactory, typename... Args>
        Fiber(CoroutineFactory&& coroutine_factory, Args&&... args);

        [[nodiscard]] Event& completion() override;
        [[nodiscard]] const Event& completion() const override;
        [[nodiscard]] T& value();
        [[nodiscard]] const T& value() const;

    private:
        Coroutine<T> main_coroutine_;
    };

    // TEMP TEMP TEMP
    template<typename T>
    class FutureAwaitable : public AwaitableBase {
    public:
        FutureAwaitable(Future<T>& future);

        [[nodiscard]] T await_resume();

    private:
        Future<T>& future_;
    };

    template<typename T>
    class CoroutineAwaitable : public AwaitableBase {
    public:
        CoroutineAwaitable(Coroutine<T> coroutine);

        [[nodiscard]] T await_resume();

    private:
        Coroutine<T> coroutine_;
    };

    template<typename T>
    class FiberAwaitable : public AwaitableBase {
    public:
        FiberAwaitable(Fiber<T>& fiber);

        [[nodiscard]] T& await_resume();

    private:
        Fiber<T>& fiber_;
    };

    template<typename T>
    inline FutureAwaitable<T> operator co_await(Future<T>& future) {
        return FutureAwaitable<T>{future};
    }

    template<typename T>
    inline CoroutineAwaitable<T> operator co_await(Coroutine<T> coroutine) {
        return CoroutineAwaitable<T>{std::move(coroutine)};
    }

    template<typename T>
    inline FiberAwaitable<T> operator co_await(Fiber<T>& fiber) {
        return FiberAwaitable<T>{fiber};
    }

}

#include "slag/fiber.hpp"
