#include <new>

template<typename T>
slag::Result<T>::Result(T&& value)
    : error_{ErrorCode::SUCCESS}
{
    new(&value_storage_[0]) T{std::move(value)};
}

template<typename T>
slag::Result<T>::Result(const T& value)
    : error_{ErrorCode::SUCCESS}
{
    new(&value_storage_[0]) T{value};
}

template<typename T>
slag::Result<T>::Result(Error error)
    : error_{error}
{
    if (!has_error()) {
        abort();
    }
}

template<typename T>
slag::Result<T>::Result(Result&& other)
    : error_{other.error_}
{
    if (has_value()) {
        new(&value_storage_[0]) T{std::move(other.value())};
    }
}

template<typename T>
slag::Result<T>::Result(const Result& other)
    : error_{other.error_}
{
    if (has_value()) {
        new(&value_storage_[0]) T{std::move(other.value())};
    }
}

template<typename T>
slag::Result<T>::~Result() {
    cleanup();
}

template<typename T>
slag::Result<T>& slag::Result<T>::operator=(Result&& that) {
    if (this != &that) {
        cleanup();

        error_ = that.error_;
        if (has_value()) {
            new(&value_storage_[0]) T{std::move(that.value())};
        }
    }

    return *this;
}

template<typename T>
slag::Result<T>& slag::Result<T>::operator=(const Result& that) {
    if (this != &that) {
        cleanup();

        error_ = that.error_;
        if (has_value()) {
            new(&value_storage_[0]) T{that.value()};
        }
    }

    return *this;
}

template<typename T>
bool slag::Result<T>::has_value() const {
    return error_.code() == ErrorCode::SUCCESS;
}

template<typename T>
bool slag::Result<T>::has_error() const {
    return !has_value();
}

template<typename T>
T& slag::Result<T>::value() {
    assert(has_value());
    return *reinterpret_cast<T*>(&value_storage_[0]);
}

template<typename T>
const T& slag::Result<T>::value() const {
    assert(has_value());
    return *reinterpret_cast<const T*>(&value_storage_[0]);
}

template<typename T>
slag::Error& slag::Result<T>::error() {
    assert(has_error());
    return error_;
}

template<typename T>
const slag::Error& slag::Result<T>::error() const {
    assert(has_error());
    return error_;
}

template<typename T>
void slag::Result<T>::cleanup() {
    if (has_value()) {
        value().~T();
    }
}
