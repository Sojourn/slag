#include <new>
#include <cassert>

template<typename T>
slag::Result<T>::Result(T&& value)
    : contents_{Contents::VALUE}
{
    new(&storage_) T{std::move(value)};
}

template<typename T>
slag::Result<T>::Result(const T& value)
    : contents_{Contents::VALUE}
{
    new(&storage_) T{value};
}

template<typename T>
slag::Result<T>::Result(Error error)
    : contents_{Contents::ERROR}
{
    new(&storage_) Error{error};
}

template<typename T>
slag::Result<T>::Result(Result&& other)
    : contents_{other.contents_}
{
    switch (contents_) {
        case Contents::VALUE: {
            new(&storage_) T{std::move(other.value())};
            break;
        }
        case Contents::ERROR: {
            new(&storage_) Error{std::move(other.error())};
            break;
        }
    }
}

template<typename T>
slag::Result<T>::Result(const Result& other)
    : contents_{other.contents_}
{
    switch (contents_) {
        case Contents::VALUE: {
            new(&storage_) T{other.value()};
            break;
        }
        case Contents::ERROR: {
            new(&storage_) Error{other.error()};
            break;
        }
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

        contents_ = that.contents_;
        switch (contents_) {
            case Contents::VALUE: {
                new(&storage_) T{std::move(that.value())};
                break;
            }
            case Contents::ERROR: {
                new(&storage_) Error{std::move(that.error())};
                break;
            }
        }
    }

    return *this;
}

template<typename T>
slag::Result<T>& slag::Result<T>::operator=(const Result& that) {
    if (this != &that) {
        cleanup();

        contents_ = that.contents_;
        switch (contents_) {
            case Contents::VALUE: {
                new(&storage_) T{that.value()};
                break;
            }
            case Contents::ERROR: {
                new(&storage_) Error{that.error()};
                break;
            }
        }
    }

    return *this;
}

template<typename T>
bool slag::Result<T>::has_value() const {
    return contents_ == Contents::VALUE;
}

template<typename T>
bool slag::Result<T>::has_error() const {
    return contents_ == Contents::ERROR;
}

template<typename T>
T& slag::Result<T>::value() {
    assert(has_value());
    return *reinterpret_cast<T*>(&storage_);
}

template<typename T>
const T& slag::Result<T>::value() const {
    assert(has_value());
    return *reinterpret_cast<const T*>(&storage_);
}

template<typename T>
slag::Error& slag::Result<T>::error() {
    assert(has_error());
    return *reinterpret_cast<Error*>(&storage_);
}

template<typename T>
const slag::Error& slag::Result<T>::error() const {
    assert(has_error());
    return *reinterpret_cast<const Error*>(&storage_);
}

template<typename T>
void slag::Result<T>::cleanup() {
    switch (contents_) {
        case Contents::VALUE: {
            value().~T();
            break;
        }
        case Contents::ERROR: {
            error().~Error();
            break;
        }
    }
}
