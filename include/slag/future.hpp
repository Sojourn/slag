#include <new>

template<typename T>
inline slag::FutureContext<T>::FutureContext()
    : promise_attached_{false}
    , future_attached_{false}
    , promise_broken_{false}
    , promise_satisfied_{false}
    , future_retrieved_{false}
    , result_{}
{
}

template<typename T>
inline bool slag::FutureContext<T>::is_referenced() const {
    return promise_attached_ || future_attached_;
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
inline slag::Event& slag::FutureContext<T>::event() {
    return event_;
}

template<typename T>
inline const slag::Event& slag::FutureContext<T>::event() const {
    return event_;
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
    assert(!promise_attached_);
    assert(!promise_satisfied_);
    assert(!promise_broken_);

    try {
        Error{ErrorCode::PROMISE_BROKEN}.raise("");
    }
    catch (const std::exception&) {
        result_ = std::current_exception();
    }

    promise_broken_ = true;
    event_.set();
}

template<typename T>
inline void slag::FutureContext<T>::handle_future_retrieved() {
    assert(future_attached_);
    assert(!future_retrieved_);

    future_retrieved_ = true;
}

template<typename T>
inline void slag::FutureContext<T>::handle_promise_satisfied() {
    assert(promise_attached_);
    assert(!promise_satisfied_);
    assert(!promise_broken_);

    promise_satisfied_ = true;
    event_.set();
}

template<typename T>
inline void slag::FutureContext<T>::attach(Promise<T>& promise) {
    assert(!promise_attached_);

    (void)promise; // add a moved function if we ever care about this

    promise_attached_ = true;
}

template<typename T>
inline void slag::FutureContext<T>::detach(Promise<T>& promise) {
    assert(promise_attached_);

    (void)promise; // add a moved function if we ever care about this

    promise_attached_ = false;
    if (future_attached_ && !promise_satisfied_) {
        handle_promise_broken();
    }
}

template<typename T>
inline void slag::FutureContext<T>::attach(Future<T>& future) {
    if (future_retrieved_) {
        Error{ErrorCode::FUTURE_ALREADY_RETRIEVED}.raise("Failed attach future");
    }
    assert(!future_attached_);
    assert(!future_retrieved_);

    (void)future; // add a moved function if we ever care about this

    future_attached_ = true;
    handle_future_retrieved();
}

template<typename T>
inline void slag::FutureContext<T>::detach(Future<T>& future) {
    assert(future_attached_);
    assert(future_retrieved_);

    (void)future; // add a moved function if we ever care about this

    future_attached_ = false;
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
    }

    return *this;
}

template<typename T>
inline slag::Future<T> slag::Promise<T>::get_future() {
    return Future<T>{get_context()};
}

template<typename T>
inline slag::Event& slag::Promise<T>::event() {
    return get_context().event();
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
    }

    return *this;
}

template<typename T>
inline bool slag::Future<T>::is_ready() const {
    return get_context().is_promise_satisfied();
}

template<typename T>
inline slag::Event& slag::Future<T>::event() {
    return get_context().event();
}

template<typename T>
inline const slag::Event& slag::Future<T>::event() const {
    return get_context().event();
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
