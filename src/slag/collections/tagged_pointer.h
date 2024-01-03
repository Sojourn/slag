#pragma once

#include <cstdint>
#include <cstddef>

namespace slag {

    template<typename T, size_t tag_bits = 3>
    class TaggedPointer {
        static constexpr uintptr_t TAG_MASK = (1ull << tag_bits) - 1;
        static constexpr uintptr_t POINTER_MASK = ~TAG_MASK;

    public:
        TaggedPointer();
        explicit TaggedPointer(T* pointer, size_t tag = 0);
        TaggedPointer(const TaggedPointer&) noexcept = default;

        TaggedPointer& operator=(T* pointer);
        TaggedPointer& operator=(const TaggedPointer&) = default;

        [[nodiscard]] T* pointer();
        [[nodiscard]] const T* pointer() const;
        void set_pointer(T* pointer);

        [[nodiscard]] size_t tag() const;
        void set_tag(size_t tag);

        [[nodiscard]] T* operator->();
        [[nodiscard]] const T* operator->() const;
        [[nodiscard]] T& operator*();
        [[nodiscard]] const T& operator*() const;
        [[nodiscard]] bool operator==(const T* rhs) const;
        [[nodiscard]] bool operator!=(const T* rhs) const;

        explicit operator bool() const {
            return static_cast<bool>(pointer());
        }

    private:
        uintptr_t value_;
    };

}

#include "tagged_pointer.hpp"
