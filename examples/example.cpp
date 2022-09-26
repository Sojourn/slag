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

template<typename T, typename Promise>
class AwaitableFuture
    : private Task
    , private EventObserver
{
public:
    AwaitableFuture(Future<T>& future)
        : future_{future}
    {
    }

    [[nodiscard]] bool await_ready() {
        return future_.event().is_set();
    }

    void await_suspend(std::coroutine_handle<Promise> handle) {
        handle_ = handle;
        EventObserver::wait(future_.event(), nullptr);
    }

    [[nodiscard]] T await_resume() {
        if (future_.result().has_error()) {
            future_.result().error().raise("FutureError");
        }

        return std::move(future_.result().value());
    }

private:
    void run() override {
        handle_.resume();
    }

    void handle_event_set(Event& event, void* user_data) override {
        (void)event;
        (void)user_data;

        Task::schedule();
    }

    void handle_event_destroyed(void* user_data) override {
        (void)user_data;

        Task::schedule();
    }

private:
    Future<T>&                     future_;
    std::coroutine_handle<Promise> handle_;
};

template<typename T>
class Coroutine {public:
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

        template<typename U>
        [[nodiscard]] AwaitableFuture<U, promise_type> await_transform(Future<U>& future) {
            return AwaitableFuture<U, promise_type>{future};
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

    T operator()() {
        return std::move(handle_.promise().get_value());
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

Future<int> make_future(int value) {
    Promise<int> promise;
    promise.set_value(value);
    return promise.get_future();
}

Coroutine<int> do_stuff() {
    Future<int> future = make_future(13);
    int value = co_await future;

    co_return 15 + value;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    Coroutine<int> foo = do_stuff();

    while (!foo.is_done()) {
        foo.resume();
    }

    std::cout << foo.get_value() << std::endl;

    // event_loop.run();

    return 0;
}
