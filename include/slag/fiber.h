#pragma once

#include <variant>
#include <coroutine>
#include <cassert>
#include "slag/task.h"
#include "slag/fiber_base.h"
#include "slag/awaitable.h"
#include "slag/coroutine.h"

namespace slag {

    template<typename T>
    class Fiber : public FiberBase {
    public:
        template<typename CoroutineFactory, typename... Args>
        Fiber(CoroutineFactory&& coroutine_factory, Args&&... args);

        [[nodiscard]] T& value();
        [[nodiscard]] const T& value() const;

    private:
        Coroutine<T> main_coroutine_;
    };

#if 0

    template<typename T>
    class FiberAwaitable : public Awaitable {
    public:
        FiberAwaitable(Fiber<T>& fiber)
            : Awaitable{fiber, PollableEvent::READABLE}
            , fiber_{fiber}
        {
        }

        [[nodiscard]] T& await_resume() {
            return fiber_.value();
        }

    private:
        Fiber<T>& fiber_;
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

#endif

}

#include "slag/fiber.hpp"
