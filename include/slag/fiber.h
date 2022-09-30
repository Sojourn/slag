#pragma once

#include <variant>
#include <coroutine>
#include <cassert>
#include "slag/task.h"
#include "slag/event.h"
#include "slag/future.h"
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

    template<typename T>
    class Coroutine {
    public:
        class Promise {
        public:
            Coroutine<T> get_return_object() noexcept {
                return Coroutine<T>{*this};
            }

            std::suspend_always initial_suspend() const noexcept {
                return {};
            }

            std::suspend_always final_suspend() const noexcept {
                return {};
            }

            std::suspend_always yield_value(const T&) noexcept {
                throw std::runtime_error("Coroutine cannot yield");
            }

            void return_value(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
                result_ = std::move(value);
            }

            void return_value(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
                result_ = value;
            }

            void unhandled_exception() noexcept(std::is_nothrow_copy_constructible_v<std::exception_ptr>) {
                result_ = std::current_exception();
            }

            [[nodiscard]] bool is_done() const noexcept {
                return result_.index() > 0; // TODO: look if there is a cleaner way to do this
            }

            [[nodiscard]] T& get_value() {
                if (std::holds_alternative<std::exception_ptr>(result_)) {
                    std::rethrow_exception(std::get<std::exception_ptr>(result_));
                }

                return std::get<T>(result_);
            }

            template<typename U>
            [[nodiscard]] FutureAwaitable<U, Promise> await_transform(Future<U>& future) {
                return FutureAwaitable<U, Promise>{future};
            }

        private:
            std::variant<std::monostate, T, std::exception_ptr> result_;
        };

        using promise_type = Promise;

    public:
        Coroutine() = default;

        ~Coroutine() {
            if (handle_) {
                handle_.destroy();
            }
        }

        Coroutine(Coroutine&& other) noexcept
            : handle_{std::exchange(other.handle_, nullptr)}
        {
        }

        Coroutine& operator=(Coroutine&& that) noexcept {
            if (this != &that) {
                if (handle_) {
                    handle_.destroy();
                }

                handle_ = std::exchange(that.handle_, nullptr);
            }

            return *this;
        }

        void resume() {
            handle_();
        }

        [[nodiscard]] explicit operator bool() const {
            return static_cast<bool>(handle_);
        }

        [[nodiscard]] bool is_done() const noexcept {
            return handle_.promise().is_done();
        }

        [[nodiscard]] T& get_value() noexcept {
            return handle_.promise().get_value();
        }

        [[nodiscard]] std::coroutine_handle<> handle() {
            return handle_;
        }

        T operator()() {
            return std::move(handle_.promise().get_value());
        }

    private:
        explicit Coroutine(Promise& promise) noexcept
            : handle_{std::coroutine_handle<Promise>::from_promise(promise)}
        {
        }

    private:
        std::coroutine_handle<Promise> handle_;
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
