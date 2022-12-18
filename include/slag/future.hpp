#include <new>

template<typename T>
inline slag::FutureContext<T>::FutureContext()
    : promise_{nullptr}
    , future_{nullptr}
    , result_{}
    , promise_broken_{false}
    , promise_satisfied_{false}
    , future_retrieved_{false}
{
}

template<typename T>
inline bool slag::FutureContext<T>::is_referenced() const {
    return promise_ || future_;
}

template<typename T>
inline bool slag::FutureContext<T>::is_promise_satisfied() const {
    return promise_satisfied_;
}

template<typename T>
inline bool slag::FutureContext<T>::is_future_retrieved() const {
    return future_retrieved_;
}

template<typename T>
inline auto slag::FutureContext<T>::result() -> Result& {
    return result_;
}

template<typename T>
inline auto slag::FutureContext<T>::result() const -> const Result& {
    return result_;
}

template<typename T>
inline void slag::FutureContext<T>::handle_promise_broken() {
    assert(!promise_);
    assert(!promise_satisfied_);
    assert(!promise_broken_);

    try {
        Error{ErrorCode::PROMISE_BROKEN}.raise("");
    }
    catch (const std::exception&) {
        result_ = std::current_exception();
    }

    promise_broken_ = true;
    if (future_) {
        future_->set_event(PollableEvent::READABLE);
    }
}

template<typename T>
inline void slag::FutureContext<T>::handle_future_retrieved() {
    assert(future_);
    assert(!future_retrieved_);

    future_retrieved_ = true;
}

template<typename T>
inline void slag::FutureContext<T>::handle_promise_satisfied() {
    assert(promise_);
    assert(!promise_satisfied_);
    assert(!promise_broken_);

    promise_satisfied_ = true;
    if (future_) {
        future_->set_event(PollableEvent::READABLE);
    }
}

template<typename T>
inline void slag::FutureContext<T>::attach(Promise<T>& promise) {
    assert(!promise_);

    promise_ = &promise;
}

template<typename T>
inline void slag::FutureContext<T>::detach(Promise<T>& promise) {
    assert(promise_);
    assert(promise_ == &promise);

    promise_ = nullptr;
    if (future_ && !promise_satisfied_) {
        handle_promise_broken();
    }
}

template<typename T>
inline void slag::FutureContext<T>::moved(Promise<T>& old_promise, Promise<T>& new_promise) {
    assert(promise_ == &old_promise);
    assert(&old_promise != &new_promise);

    promise_ = &new_promise;
}

template<typename T>
inline void slag::FutureContext<T>::attach(Future<T>& future) {
    if (future_retrieved_) {
        Error{ErrorCode::FUTURE_ALREADY_RETRIEVED}.raise("Failed attach future");
    }
    assert(!future_);

    future_ = &future;
    handle_future_retrieved();
}

template<typename T>
inline void slag::FutureContext<T>::detach(Future<T>& future) {
    assert(future_ == &future);
    assert(future_retrieved_);

    future_ = nullptr;
}

template<typename T>
inline void slag::FutureContext<T>::moved(Future<T>& old_future, Future<T>& new_future) {
    assert(future_ == &old_future);
    assert(&old_future != &new_future);

    future_ = &new_future;
}

template<typename T>
inline slag::Promise<T>::Promise()
    : context_{nullptr}
{
}

template<typename T>
inline slag::Promise<T>::Promise(Promise&& other)
    : context_{std::exchange(other.context_, nullptr)}
{
    if (context_) {
        context_->moved(other, *this);
    }
}

template<typename T>
inline slag::Promise<T>::~Promise() {
    reset();
}

template<typename T>
inline slag::Promise<T>& slag::Promise<T>::operator=(Promise&& that) {
    if (this != &that) {
        reset();
        context_ = std::exchange(that.context_, nullptr);
        if (context_) {
            context_->moved(that, *this);
        }
    }

    return *this;
}

template<typename T>
inline slag::Future<T> slag::Promise<T>::get_future() {
    return Future<T>{get_context()};
}

template<typename T>
inline void slag::Promise<T>::set_value(T&& value) {
    FutureContext<T>& context = get_context();
    if (context.is_promise_satisfied()) {
        Error{ErrorCode::PROMISE_ALREADY_SATISFIED}.raise("Failed to set future value");
    }

    context.result() = std::move(value);
    context.handle_promise_satisfied();
}

template<typename T>
inline void slag::Promise<T>::set_value(const T& value) {
    FutureContext<T>& context = get_context();
    if (context.is_promise_satisfied()) {
        Error{ErrorCode::PROMISE_ALREADY_SATISFIED}.raise("Failed to set future value");
    }

    context.result() = value;
    context.handle_promise_satisfied();
}

template<typename T>
inline void slag::Promise<T>::set_error(Error error, std::string_view message) {
    try {
        error.raise(message);
    }
    catch (const std::exception&) {
        set_current_exception();
    }
}

template<typename T>
inline void slag::Promise<T>::set_exception(std::exception_ptr ex) {
    FutureContext<T>& context = get_context();
    if (context.is_promise_satisfied()) {
        Error{ErrorCode::PROMISE_ALREADY_SATISFIED}.raise("Failed to set future error");
    }

    context.result() = std::move(ex);
    context.handle_promise_satisfied();
}

template<typename T>
inline void slag::Promise<T>::set_current_exception() {
    set_exception(std::current_exception());
}

template<typename T>
inline void slag::Promise<T>::reset() {
    if (context_) {
        context_->detach(*this);
        if (!context_->is_referenced()) {
            delete context_;
        }

        context_ = nullptr;
    }
}

template<typename T>
inline slag::FutureContext<T>& slag::Promise<T>::get_context() {
    if (!context_) {
        context_ = new FutureContext<T>; // TODO: use custom allocator(s)
        context_->attach(*this);
    }

    return *context_;
}

template<typename T>
inline slag::Future<T>::Future()
    : context_{nullptr}
{
}

template<typename T>
inline slag::Future<T>::Future(Future&& other)
    : context_{std::exchange(other.context_, nullptr)}
{
    if (context_) {
        context_->moved(other, *this);
    }
}

template<typename T>
inline slag::Future<T>::~Future() {
    reset();
}

template<typename T>
inline slag::Future<T>& slag::Future<T>::operator=(Future&& that) {
    if (this != &that) {
        reset();
        context_ = std::exchange(that.context_, nullptr);
        if (context_) {
            context_->moved(that, *this);
        }
    }

    return *this;
}

template<typename T>
inline bool slag::Future<T>::is_ready() const {
    return get_context().is_promise_satisfied();
}

template<typename T>
inline T& slag::Future<T>::get() {
    auto&& context = get_context();
    if (!context.is_promise_satisfied()) {
        Error{ErrorCode::FUTURE_NOT_READY}.raise("Failed to get result");
    }

    auto&& result = context.result();
    if (auto ex = std::get_if<std::exception_ptr>(&result)) {
        std::rethrow_exception(*ex);
    }

    return std::get<T>(result);
}

template<typename T>
inline const T& slag::Future<T>::get() const {
    auto&& context = get_context();
    if (!context.is_promise_satisfied()) {
        Error{ErrorCode::FUTURE_NOT_READY}.raise("Failed to get result");
    }

    auto&& result = context.result();
    if (auto ex = std::get_if<std::exception_ptr>(&result)) {
        std::rethrow_exception(*ex);
    }

    return std::get<T>(result);
}

template<typename T>
inline void slag::Future<T>::reset() {
    if (context_) {
        context_->detach(*this);
        if (!context_->is_referenced()) {
            delete context_;
        }

        context_ = nullptr;
    }
}

template<typename T>
inline slag::Future<T>::Future(FutureContext<T>& context)
    : context_{&context}
{
    context_->attach(*this);
}

template<typename T>
inline slag::FutureContext<T>& slag::Future<T>::get_context() {
    if (!context_) {
        Error{ErrorCode::FUTURE_DETACHED}.raise("Failed to get the future context");
    }

    return *context_;
}

template<typename T>
inline const slag::FutureContext<T>& slag::Future<T>::get_context() const {
    if (!context_) {
        Error{ErrorCode::FUTURE_DETACHED}.raise("Failed to get the future context");
    }

    return *context_;
}
