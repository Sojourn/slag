#include <cassert>


template<typename T>
slag::FutureContext<T>::FutureContext()
    : promise_attached_{false}
    , future_attached_{false}
    , promise_broken_{false}
    , promise_satisfied{false}
    , future_retrieved_{false}
{
}

template<typename T>
slag::Result<T>& slag::FutureContext<T>::result() {
    return result_;
}

template<typename T>
const slag::Result<T>& slag::FutureContext<T>::result() const {
    return result_;
}

template<typename T>
void slag::FutureContext<T>::handle_promise_broken() {
    assert(!promise_attached_);
    assert(!promise_satisfied_);
    assert(!promise_broken_);

    promise_broken_ = true;
    result_.set_error(Error{ErrorCode::PROMISE_BROKEN});
    event_.set();
}

template<typename T>
void slag::FutureContext<T>::handle_future_retrieved() {
    assert(future_attached_);
    assert(!future_retrieved_);

    future_retrieved_ = true;
}

template<typename T>
void slag::FutureContext<T>::handle_promise_satisfied() {
    assert(promise_attached_);
    assert(!promise_satisfied_);
    assert(!promise_broken_);

    promise_satisfied_ = true;
    event_.set();
}

template<typename T>
void slag::FutureContext<T>::attach(Promise&) {
    assert(!promise_attached_);

    promise_attached_ = true;
}

template<typename T>
void slag::FutureContext<T>::detach(Promise&) {
    assert(promise_attached_);

    promise_attached_ = false;
    if (future_attached_ && !promise_satisfied_) {
        handle_promise_broken();
    }
}

template<typename T>
void slag::FutureContext<T>::attach(Future&) {
    assert(!future_attached_);
    assert(!future_retrieved_);

    future_attached_ = true;
    handle_future_retrieved();
}

template<typename T>
void slag::FutureContext<T>::detach(Future&) {
    assert(future_attached_);
    assert(future_retrieved_);

    future_attached_ = false;
}

template<typename T>
bool slag::FutureContext<T>::is_referenced() const {
    return promise_attached_ || future_attached_;
}
