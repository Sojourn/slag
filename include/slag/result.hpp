#include <new>
#include <cstdlib>
#include <cassert>

template<typename T>
slag::Result<T>::Result(T&& value)
    : error_{ErrorCode::SUCCESS}
{
    new(&value_storage_) T{std::move(value)};
}

template<typename T>
slag::Result<T>::Result(const T& value)
    : error_{ErrorCode::SUCCESS}
{
    new(&value_storage_) T{value};
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
        new(&value_storage_) T{std::move(other.value())};
    }
}

template<typename T>
slag::Result<T>::Result(const Result& other)
    : error_{other.error_}
{
    if (has_value()) {
        new(&value_storage_) T{std::move(other.value())};
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
            new(&value_storage_) T{std::move(that.value())};
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
            new(&value_storage_) T{that.value()};
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
    return *reinterpret_cast<T*>(&value_storage_);
}

template<typename T>
const T& slag::Result<T>::value() const {
    assert(has_value());
    return *reinterpret_cast<const T*>(&value_storage_);
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

template<>
slag::Result<void>::Result()
    : error_{ErrorCode::SUCCESS}
{
}

template<>
slag::Result<void>::Result(Error error)
    : error_{error}
{
    if (!has_error()) {
        abort();
    }
}

template<>
slag::Result<void>::Result(const Result& other)
    : error_{other.error_}
{
}

template<>
slag::Result<void>& slag::Result<void>::operator=(const Result& that) {
    if (this != &that) {
        error_ = that.error_;
    }

    return *this;
}

template<>
bool slag::Result<void>::has_value() const {
    return error_.code() == ErrorCode::SUCCESS;
}

template<>
bool slag::Result<void>::has_error() const {
    return !has_value();
}

template<>
slag::Error& slag::Result<void>::error() {
    assert(has_error());
    return error_;
}

template<>
const slag::Error& slag::Result<void>::error() const {
    assert(has_error());
    return error_;
}
