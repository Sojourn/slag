#include <iostream>
#include <any>
#include <span>
#include <memory>
#include <vector>
#include <functional>
#include <coroutine>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include "slag/slag.h"

using namespace slag;

template<typename T>
class Coroutine {
public:
    class promise_type {
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

        std::suspend_always yield_value(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            abort(); // not supported
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

#if 0
        [[nodiscard]] std::suspend_always await_transform(int expr) {
            (void)expr;
            std::cout << expr << std::endl;
            return {};
        }
#endif

    private:
        std::variant<std::monostate, T, std::exception_ptr> result_;
    };

private:
    explicit Coroutine(promise_type& promise) noexcept
        : handle_{std::coroutine_handle<promise_type>::from_promise(promise)}
    {
    }

public:
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

    T operator()() {
        return std::move(handle_.promise().get_value());
    }

    [[nodiscard]] bool await_ready() {
        return !is_done();
    }

    void await_suspend(std::coroutine_handle<> h) {
        h.resume();
    }

    void await_resume() {
        handle_.resume();
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

Coroutine<int> do_foo() {
    co_return 3;
}

Coroutine<int> do_stuff() {
    auto coro = do_foo();
    co_await coro;

    co_return 15 + coro();
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Coroutine<int> foo = do_stuff();
    assert(foo);

    while (!foo.is_done()) {
        foo.resume();
    }

    std::cout << foo.get_value() << std::endl;

    // EventLoop event_loop{std::make_unique<IOURingReactor>()};
    // event_loop.run();

    return 0;
}
