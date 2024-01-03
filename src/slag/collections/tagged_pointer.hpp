#include <cassert>

namespace slag {

    template<typename T, size_t tag_bits>
    inline TaggedPointer<T, tag_bits>::TaggedPointer()
        : value_{0}
    {
    }

    template<typename T, size_t tag_bits>
    inline TaggedPointer<T, tag_bits>::TaggedPointer(T* pointer, size_t tag)
        : value_{(reinterpret_cast<uintptr_t>(pointer) & POINTER_MASK) | (tag & TAG_MASK)}
    {
        assert((reinterpret_cast<uintptr_t>(pointer) & TAG_MASK) == 0);
        assert(tag <= TAG_MASK);
    }

    template<typename T, size_t tag_bits>
    inline TaggedPointer<T, tag_bits>& slag::TaggedPointer<T, tag_bits>::operator=(T* pointer) {
        set_pointer(pointer);
        return *this;
    }

    template<typename T, size_t tag_bits>
    inline T* TaggedPointer<T, tag_bits>::pointer() {
        return reinterpret_cast<T*>(value_ & POINTER_MASK);
    }

    template<typename T, size_t tag_bits>
    inline const T* TaggedPointer<T, tag_bits>::pointer() const {
        return reinterpret_cast<const T*>(value_ & POINTER_MASK);
    }

    template<typename T, size_t tag_bits>
    inline void TaggedPointer<T, tag_bits>::set_pointer(T* pointer) {
        assert((reinterpret_cast<uintptr_t>(pointer) & TAG_MASK) == 0);
        value_ = (reinterpret_cast<uintptr_t>(pointer) & POINTER_MASK) | (value_ & TAG_MASK);
    }

    template<typename T, size_t tag_bits>
    inline size_t TaggedPointer<T, tag_bits>::tag() const {
        return static_cast<size_t>(value_ & TAG_MASK);
    }

    template<typename T, size_t tag_bits>
    inline void TaggedPointer<T, tag_bits>::set_tag(size_t tag) {
        assert(tag <= TAG_MASK);
        value_ = (value_ & POINTER_MASK) | (tag & TAG_MASK);
    }

    template<typename T, size_t tag_bits>
    inline T* TaggedPointer<T, tag_bits>::operator->() {
        return pointer();
    }

    template<typename T, size_t tag_bits>
    inline const T* TaggedPointer<T, tag_bits>::operator->() const {
        return pointer();
    }

    template<typename T, size_t tag_bits>
    inline T& TaggedPointer<T, tag_bits>::operator*() {
        assert(*this);
        return *pointer();
    }

    template<typename T, size_t tag_bits>
    inline const T& TaggedPointer<T, tag_bits>::operator*() const {
        assert(*this);
        return *pointer();
    }

    template<typename T, size_t tag_bits>
    inline bool TaggedPointer<T, tag_bits>::operator==(const T* rhs) const {
        return pointer() == rhs;
    }

    template<typename T, size_t tag_bits>
    inline bool TaggedPointer<T, tag_bits>::operator!=(const T* rhs) const {
        return !operator==(rhs);
    }

}
