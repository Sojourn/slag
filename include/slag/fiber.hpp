#include <cassert>
#include "slag/logging.h"

namespace slag {

    template<typename T>
    template<typename CoroutineFactory, typename... Args>
    Fiber<T>::Fiber(CoroutineFactory&& coroutine_factory, Args&&... args) {
        // null activation to prevent the coroutine from resuming on creation
        Activation activation;
        (void)activation;

        main_coroutine_ = coroutine_factory(std::forward<Args>(args)...);
        to_pollable(main_coroutine_).add_observer(*this);
        resume(main_coroutine_.handle(), TaskPriority::HIGH);
    }

    template<typename T>
    Fiber<T>::~Fiber() {
        to_pollable(main_coroutine_).remove_observer(*this);
    }

    template<typename T>
    T& Fiber<T>::value() {
        return main_coroutine_.value();
    }

    template<typename T>
    const T& Fiber<T>::value() const {
        return main_coroutine_.value();
    }

    template<typename T>
    void Fiber<T>::handle_pollable_event(Pollable& pollable, Event event) {
        assert(&pollable == &to_pollable(main_coroutine_));

        if (event == PollableEvent::READABLE) {
            set_event(event);
        }
    }

    template<typename T>
    void Fiber<T>::handle_pollable_destroyed(Pollable& pollable) {
        assert(&pollable == &to_pollable(main_coroutine_));

        abort();
    }

}

