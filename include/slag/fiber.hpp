#include <cassert>
#include "slag/logging.h"

template<typename T>
template<typename F, typename... Args>
slag::Fiber<T>::Fiber(F&& f, Args&&... args) {
    Activation activation{*this};
    main_coroutine_ = f(std::forward<Args>(args)...);
    pending_coroutine_ = main_coroutine_.handle();
    schedule(TaskPriority::HIGH);
}

template<typename T>
slag::Future<T> slag::Fiber<T>::get_future() {
    return promise_.get_future();
}

template<typename T>
void slag::Fiber<T>::run() {
    Activation activation{*this};
    std::exchange(pending_coroutine_, {}).resume();
    if (main_coroutine_.is_done()) {
        promise_.set_value(std::move(main_coroutine_.value()));
    }
}

template<typename T>
slag::FutureAwaitable<T>::FutureAwaitable(Future<T>& future)
    : future_{future}
{
}

template<typename T>
bool slag::FutureAwaitable<T>::await_ready() const noexcept {
    return future_.event().is_set(); // might not need to suspend
}

template<typename T>
void slag::FutureAwaitable<T>::await_suspend(std::coroutine_handle<> handle) {
    FiberBase* active_fiber = local_event_loop().active_fiber();
    assert(active_fiber);

    active_fiber->set_pending_coroutine(handle);
    wait(future_.event(), active_fiber);
}

template<typename T>
T slag::FutureAwaitable<T>::await_resume() {
    auto&& result = future_.result();
    if (result.has_error()) {
        result.error().raise("FutureError");
    }

    return std::move(result.value());
}

template<typename T>
void slag::FutureAwaitable<T>::handle_event_set(Event& event, void* user_data) {
    assert(event.is_set());
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}

template<typename T>
void slag::FutureAwaitable<T>::handle_event_destroyed(void* user_data) {
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}

template<typename T>
slag::CoroutineAwaitable<T>::CoroutineAwaitable(Coroutine<T> coroutine)
    : coroutine_{std::move(coroutine)}
    , future_{coroutine_.get_future()}
{
}

template<typename T>
bool slag::CoroutineAwaitable<T>::await_ready() const noexcept {
    return future_.event().is_set(); // might not need to suspend
}

template<typename T>
void slag::CoroutineAwaitable<T>::await_suspend(std::coroutine_handle<> handle) {
    FiberBase* active_fiber = local_event_loop().active_fiber();
    assert(active_fiber);

    active_fiber->set_pending_coroutine(handle);
    wait(future_.event(), active_fiber);
}

template<typename T>
T slag::CoroutineAwaitable<T>::await_resume() {
    auto&& result = future_.result();
    if (result.has_error()) {
        result.error().raise("FutureError");
    }

    return std::move(result.value());
}

template<typename T>
void slag::CoroutineAwaitable<T>::handle_event_set(Event& event, void* user_data) {
    assert(event.is_set());
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}

template<typename T>
void slag::CoroutineAwaitable<T>::handle_event_destroyed(void* user_data) {
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}

template<typename T>
slag::FiberAwaitable<T>::FiberAwaitable(Fiber<T>& fiber)
    : future_{fiber.get_future()}
{
}

template<typename T>
bool slag::FiberAwaitable<T>::await_ready() const noexcept {
    return future_.event().is_set(); // might not need to suspend
}

template<typename T>
void slag::FiberAwaitable<T>::await_suspend(std::coroutine_handle<> handle) {
    FiberBase* active_fiber = local_event_loop().active_fiber();
    assert(active_fiber);

    active_fiber->set_pending_coroutine(handle);
    wait(future_.event(), active_fiber);
}

template<typename T>
T slag::FiberAwaitable<T>::await_resume() {
    auto&& result = future_.result();
    if (result.has_error()) {
        result.error().raise("FutureError");
    }

    return std::move(result.value());
}

template<typename T>
void slag::FiberAwaitable<T>::handle_event_set(Event& event, void* user_data) {
    assert(event.is_set());
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}

template<typename T>
void slag::FiberAwaitable<T>::handle_event_destroyed(void* user_data) {
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}
