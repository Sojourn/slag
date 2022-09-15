
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

private:
    std::coroutine_handle<promise_type> handle_;
};

Coroutine<int> do_stuff() {
    co_return 14;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Coroutine<int> foo = do_stuff();
    assert(foo);

    std::cout << foo.is_done() << std::endl;
    foo.resume();
    std::cout << foo.is_done() << std::endl;
    std::cout << foo.get_value() << std::endl;

    // EventLoop event_loop{std::make_unique<IOURingReactor>()};
    // event_loop.run();

    return 0;
}
