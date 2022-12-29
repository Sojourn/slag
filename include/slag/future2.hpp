#include <cassert>

namespace slag {

    constexpr size_t FUTURE_EMPTY_INDEX = 0;
    constexpr size_t FUTURE_VALUE_INDEX = 1;
    constexpr size_t FUTURE_ERROR_INDEX = 2;

    template<typename T>
    Future<T>::Future()
        : promise_{nullptr}
        , result_{}
    {
    }

    template<typename T>
    Future<T>::Future(Future&& that)
        : Future{}
    {
        std::exchange(to_pollable(*this), std::move(to_pollable(that)));
        std::exchange(promise_, std::move(that.promise_));
        std::exchange(result_, std::move(that.result_));
    }

    template<typename T>
    Future<T>::~Future() {
        reset();
    }

    template<typename T>
    Future<T>& Future<T>::operator=(Future&& that) {
        if (this != &that) {
            reset();

            std::exchange(to_pollable(*this), std::move(to_pollable(that)));
            std::exchange(promise_, std::move(that.promise_));
            std::exchange(result_, std::move(that.result_));
        }

        return *this;
    }

    template<typename T>
    bool Future<T>::has_result() const {
        return result_.index() != FUTURE_EMPTY_INDEX;
    }

    template<typename T>
    bool Future<T>::has_value() const {
        return result_.index() == FUTURE_VALUE_INDEX;
    }

    template<typename T>
    bool Future<T>::has_error() const {
        return result_.index() == FUTURE_ERROR_INDEX;
    }

    template<typename T>
    T& Future<T>::get() {
        switch (result_.index()) {
            case FUTURE_EMPTY_INDEX: {
                Error{ErrorCode::FUTURE_NOT_READY}.raise("Future::get");
                abort(); // unreachable
            }
            case FUTURE_VALUE_INDEX: {
                return value();
            }
            case FUTURE_ERROR_INDEX: {
                std::rethrow_exception(error());
            }
        }

        abort(); // unreachable
    }

    template<typename T>
    const T& Future<T>::get() const {
        return static_cast<Future<T>&>(*this).get();
    }

    template<typename T>
    T& Future<T>::value() {
        return std::get<FutureValue<T>>(result_);
    }

    template<typename T>
    const T& Future<T>::value() const {
        return std::get<FutureValue<T>>(result_);
    }

    template<typename T>
    std::exception_ptr Future<T>::error() const {
        return std::get<FutureError<T>>(result_);
    }

    template<typename T>
    void Future<T>::reset() {
        to_pollable(*this) = Pollable{};

        if (promise_) {
            promise_->abandon(*this);
            promise_ = nullptr;
        }

        result_ = FutureResult<T>{};
    }

    template<typename T>
    Future<T>::Future(Promise<T>& promise)
        : promise_{&promise}
    {
    }

    template<typename T>
    void Future<T>::abandon(Promise<T>& promise) {
        assert(promise_ == &promise);
        promise_ = nullptr;

        if (!has_result()) {
            auto exception = to_exception(Error(ErrorCode::PROMISE_BROKEN), "Future::abandon");
            result_.template emplace<std::exception_ptr>(std::move(exception));
        }

        set_event(PollableEvent::CLOSED);
    }

    template<typename T>
    Promise<T>::Promise()
        : future_{nullptr}
    {
    }

    template<typename T>
    Promise<T>::Promise(Promise&& that)
        : future_{nullptr}
    {
        std::exchange(future_, std::move(that.future_));
    }

    template<typename T>
    Promise<T>::~Promise() {
        reset();
    }

    template<typename T>
    Promise<T>& Promise<T>::operator=(Promise&& that) {
        if (this != &that) {
            reset();
        }

        return *this;
    }

    template<typename T>
    template<typename... Args>
    void Promise<T>::emplace_value(Args&&... args) {
        if (!future_) {
            Error{ErrorCode::FUTURE_DETACHED}.raise("Promies::emplace_value");
        }
        if (future_->has_result()) {
            Error{ErrorCode::PROMISE_ALREADY_SATISFIED}.raise("Promies::emplace_value");
        }

        auto&& result = future_->result_;
        result.template emplace<T>(std::forward<Args>(args)...);
        reset_event(PollableEvent::WRITABLE);
    }

    template<typename T>
    void Promise<T>::set_value(T value) {
        emplace_value(std::move(value));
    }

    template<typename T>
    void Promise<T>::set_error(std::exception_ptr exception) {
        if (!future_) {
            Error{ErrorCode::FUTURE_DETACHED}.raise("Promies::set_error");
        }
        if (future_->has_result()) {
            Error{ErrorCode::PROMISE_ALREADY_SATISFIED}.raise("Promies::set_error");
        }

        auto&& result = future_->result_;
        result.template emplace<std::exception_ptr>(std::move(exception));
        reset_event(PollableEvent::WRITABLE);
    }

    template<typename T>
    void Promise<T>::reset() {
        to_pollable(*this) = Pollable{};

        if (future_) {
            future_->abandon(*this);
            future_ = nullptr;
        }
    }

    template<typename T>
    Promise<T>::Promise(Future<T>& future)
        : future_{&future}
    {
        set_event(PollableEvent::WRITABLE);
    }

    template<typename T>
    void Promise<T>::abandon(Future<T>& future)     {
        assert(future_ == &future);
        future_ = nullptr;

        reset_event(PollableEvent::WRITABLE);
        set_event(PollableEvent::CLOSED);
    }

    template<typename T>
    FutureFactory<T>::FutureFactory()
        : future{promise}
        , promise{future}
    {
    }

    template<typename T>
    FutureFactory<T> make_future() {
        return {};
    }

    template<typename T>
    Pollable& to_pollable(Future<T>& future) {
        return future;
    }

    template<typename T>
    Pollable& to_pollable(Promise<T>& promise) {
        return promise;
    }

}

