#include <cassert>
#include "slag/logging.h"

template<typename T, typename Promise>
slag::FutureAwaitable<T, Promise>::FutureAwaitable(Future<T>& future)
    : future_{future}
{
}

template<typename T, typename Promise>
bool slag::FutureAwaitable<T, Promise>::await_ready() const noexcept {
    return future_.event().is_set(); // might not need to suspend
}

template<typename T, typename Promise>
void slag::FutureAwaitable<T, Promise>::await_suspend(std::coroutine_handle<Promise> handle) {
    FiberBase* active_fiber = local_event_loop().active_fiber();
    assert(active_fiber);

    active_fiber->set_pending_coroutine(handle);
    wait(future_.event(), active_fiber);
}

template<typename T, typename Promise>
T slag::FutureAwaitable<T, Promise>::await_resume() {
    auto&& result = future_.result();
    if (result.has_error()) {
        result.error().raise("FutureError");
    }

    return std::move(result.value());
}

template<typename T, typename Promise>
void slag::FutureAwaitable<T, Promise>::handle_event_set(Event& event, void* user_data) {
    assert(event.is_set());
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}

template<typename T, typename Promise>
void slag::FutureAwaitable<T, Promise>::handle_event_destroyed(void* user_data) {
    assert(user_data);

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->schedule();
}

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
    info("Fiber::run");

    Activation activation{*this};
    std::exchange(pending_coroutine_, {}).resume();
    if (main_coroutine_.is_done()) {
        info("Fiber::main complete");
        promise_.set_value(std::move(main_coroutine_.get_value()));
    }
}
