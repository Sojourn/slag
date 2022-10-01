#pragma once

#include <variant>
#include <coroutine>
#include <cassert>
#include "slag/task.h"
#include "slag/event.h"
#include "slag/future.h"
#include "slag/coroutine.h"
#include "slag/event_loop.h"

namespace slag {

    class FiberBase;

    // Resumes the currently running fiber when the future is ready
    template<typename T, typename Promise>
    class FutureAwaitable : private EventObserver {
    public:
        FutureAwaitable(Future<T>& future);

        [[nodiscard]] bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<Promise> handle);
        [[nodiscard]] T await_resume();

    private:
        void handle_event_set(Event& event, void* user_data) override;
        void handle_event_destroyed(void* user_data) override;

    private:
        Future<T>& future_;
    };

    class FiberBase : public Task {
    public:
        void set_pending_coroutine(std::coroutine_handle<> handle) {
            pending_coroutine_ = handle;
        }

    protected:
        class [[nodiscard]] Activation {
        public:
            Activation(FiberBase& fiber);
            Activation(Activation&&) = delete;
            Activation(const Activation&) = delete;
            ~Activation();

            Activation& operator=(Activation&&) = delete;
            Activation& operator=(const Activation&) = delete;

        private:
            EventLoop& event_loop_;
            FiberBase* previous_active_fiber_;
        };

        std::coroutine_handle<> pending_coroutine_;
    };

    // TODO: swap the names of Task and Fiber
    template<typename T>
    class Fiber : public FiberBase {
    public:
        template<typename F, typename... Args>
        Fiber(F&& f, Args&&... args);

        [[nodiscard]] Future<T> get_future();

    private:
        void run() override;

    private:
        Promise<T>   promise_;
        Coroutine<T> main_coroutine_;
    };

}

#include "slag/fiber.hpp"
