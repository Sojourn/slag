#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/message_fragment.h"

namespace slag {

    // TODO: optimized copy constructor
    class MessageRecordFragment {
    public:
        [[nodiscard]] bool is_full() const;
        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t capacity() const;
        [[nodiscard]] std::span<uint64_t> data();
        [[nodiscard]] std::span<const uint64_t> data() const;
        [[nodiscard]] uint64_t& operator[](size_t index);
        [[nodiscard]] const uint64_t& operator[](size_t index) const;

        void push_back(uint64_t value);
        void resize(size_t size);
        void resize_uninitialized(size_t size);
        void clear();

        [[nodiscard]] bool head() const;
        void set_head(bool head=true);

        [[nodiscard]] bool tail() const;
        void set_tail(bool tail=true);

    private:
        uint8_t  size_ = 0;
        bool     head_ = false;
        bool     tail_ = false;
        uint64_t data_[MESSAGE_FRAGMENT_CAPACITY];
    };

}
