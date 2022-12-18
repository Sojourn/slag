#include <cassert>
#include "slag/fiber.h"

inline bool slag::InitialSuspend::await_ready() const noexcept {
    // Cooperates with Fiber to make the main Coroutine lazy, and other
    // Coroutines on the Fiber eager.
    //
    return static_cast<bool>(local_active_fiber());
}

inline void slag::InitialSuspend::await_suspend(std::coroutine_handle<>) const noexcept {
}

inline void slag::InitialSuspend::await_resume() const noexcept {
}

template<typename T>
slag::Coroutine<T> slag::Coroutine<T>::Promise::get_return_object() noexcept {
    return Coroutine{*this};
}

template<typename T>
slag::InitialSuspend slag::Coroutine<T>::Promise::initial_suspend() const noexcept {
    return {};
}

template<typename T>
std::suspend_always slag::Coroutine<T>::Promise::final_suspend() const noexcept {
    return {};
}

template<typename T>
std::suspend_always slag::Coroutine<T>::Promise::yield_value() const {
    throw std::runtime_error("Coroutines cannot yield");
}

template<typename T>
void slag::Coroutine<T>::Promise::return_value(T value) noexcept(std::is_nothrow_move_constructible_v<T>) {
    result_ = std::move(value);
    set_event(PollableEvent::READABLE);
}

template<typename T>
void slag::Coroutine<T>::Promise::unhandled_exception() noexcept(std::is_nothrow_copy_constructible_v<std::exception_ptr>) {
    result_ = std::current_exception();
    set_event(PollableEvent::READABLE);
}

template<typename T>
bool slag::Coroutine<T>::Promise::is_done() const noexcept {
    return result_.index() > 0;
}

template<typename T>
T& slag::Coroutine<T>::Promise::value() {
    if (std::holds_alternative<std::exception_ptr>(result_)) {
        std::rethrow_exception(
            std::get<std::exception_ptr>(result_)
        );
    }

    return std::get<T>(result_);
}

template<typename T>
const T& slag::Coroutine<T>::Promise::value() const {
    if (std::holds_alternative<std::exception_ptr>(result_)) {
        std::rethrow_exception(
            std::get<std::exception_ptr>(result_)
        );
    }

    return std::get<T>(result_);
}

template<typename T>
slag::Coroutine<T>::Coroutine(Promise& promise) noexcept
    : handle_{std::coroutine_handle<Promise>::from_promise(promise)}
{
}

template<typename T>
slag::Coroutine<T>::Coroutine(Coroutine&& other) noexcept
    : handle_{std::exchange(other.handle_, nullptr)}
{
}

template<typename T>
slag::Coroutine<T>::~Coroutine() {
    if (handle_) {
        handle_.destroy();
    }
}

template<typename T>
slag::Coroutine<T>& slag::Coroutine<T>::operator=(Coroutine&& that) noexcept {
    if (this != &that) {
        if (handle_) {
            handle_.destroy();
        }

        handle_ = std::exchange(that.handle_, nullptr);
    }

    return *this;
}

template<typename T>
bool slag::Coroutine<T>::is_done() const noexcept {
    return handle_.promise().is_done();
}

template<typename T>
T& slag::Coroutine<T>::value() noexcept {
    assert(is_done());
    return handle_.promise().value();
}

template<typename T>
const T& slag::Coroutine<T>::value() const noexcept {
    assert(is_done());
    return handle_.promise().value();
}

template<typename T>
std::coroutine_handle<> slag::Coroutine<T>::handle() noexcept {
    return handle_;
}

template<typename T>
void slag::Coroutine<T>::resume() {
    assert(!is_done());
    handle_.resume();
}
