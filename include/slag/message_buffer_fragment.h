#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/message_fragment.h"

namespace slag {

    class MessageBufferFragment {
    public:
        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t capacity() const;
        [[nodiscard]] std::span<std::byte> data();
        [[nodiscard]] std::span<const std::byte> data() const;
        [[nodiscard]] std::byte& operator[](size_t index);
        [[nodiscard]] const std::byte& operator[](size_t index) const;

        void push_back(std::byte value);
        void resize(size_t size);
        void resize_uninitialized(size_t size);
        void clear();

        [[nodiscard]] bool head() const;
        void set_head(bool head=true);

        [[nodiscard]] bool tail() const;
        void set_tail(bool tail=true);

    private:
        uint8_t   size_ = 0;
        bool      head_ = false;
        bool      tail_ = false;
        std::byte data_[MESSAGE_FRAGMENT_CAPACITY];
    };

}
