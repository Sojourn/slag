#include <cassert>
#include "slag/logging.h"

template<typename T>
template<typename CoroutineFactory, typename... Args>
slag::Fiber<T>::Fiber(CoroutineFactory&& coroutine_factory, Args&&... args) {
    // null activation to prevent the coroutine from resuming on creation
    Activation activation;
    (void)activation;

    main_coroutine_ = coroutine_factory(std::forward<Args>(args)...);
    main_coroutine_.add_observer(static_cast<Pollable::Observer&>(*this));
    resume(main_coroutine_.handle(), TaskPriority::HIGH);
}

template<typename T>
T& slag::Fiber<T>::value() {
    return main_coroutine_.value();
}

template<typename T>
const T& slag::Fiber<T>::value() const {
    return main_coroutine_.value();
}

#if 0

template<typename T>
slag::FutureAwaitable<T>::FutureAwaitable(Future<T>& future)
    : AwaitableBase{future.event()}
    , future_{future}
{
}

template<typename T>
T slag::FutureAwaitable<T>::await_resume() {
    if constexpr (std::is_same_v<T, void>) {
        future_.get();
    }
    else {
        return std::move(future_.get());
    }
}

template<typename T>
slag::CoroutineAwaitable<T>::CoroutineAwaitable(Coroutine<T> coroutine)
    : AwaitableBase{coroutine.completion()}
    , coroutine_{std::move(coroutine)}
{
}

template<typename T>
T slag::CoroutineAwaitable<T>::await_resume() {
    return std::move(coroutine_.value());
}

template<typename T>
slag::FiberAwaitable<T>::FiberAwaitable(Fiber<T>& fiber)
    : AwaitableBase{fiber.completion()}
    , fiber_{fiber}
{
}

template<typename T>
T& slag::FiberAwaitable<T>::await_resume() {
    return fiber_.value();
}

#endif
