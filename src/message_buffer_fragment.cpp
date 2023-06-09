#include "slag/message_buffer_fragment.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    bool MessageBufferFragment::is_full() const {
        return size_ == MESSAGE_FRAGMENT_CAPACITY;
    }

    bool MessageBufferFragment::is_empty() const {
        return size_ == 0;
    }

    size_t MessageBufferFragment::size() const {
        return size_;
    }

    size_t MessageBufferFragment::capacity() const {
        return MESSAGE_FRAGMENT_CAPACITY;
    }

    std::span<std::byte> MessageBufferFragment::data() {
        return std::span{data_, size_};
    }

    std::span<const std::byte> MessageBufferFragment::data() const {
        return std::span{data_, size_};
    }

    std::byte& MessageBufferFragment::operator[](size_t index) {
        assert(index < size());

        return data_[index];
    }

    const std::byte& MessageBufferFragment::operator[](size_t index) const {
        assert(index < size());

        return data_[index];
    }

    void MessageBufferFragment::push_back(std::byte value) {
        if (capacity() <= size()) {
            throw std::runtime_error("MessageBufferFragment overflow");
        }

        data_[size_++] = value;
    }

    void MessageBufferFragment::resize(size_t size) {
        if (capacity() < size) {
            throw std::runtime_error("MessageBufferFragment overflow");
        }

        size_t old_size = size_;
        size_t new_size = size;

        resize_uninitialized(new_size);

        // TODO: memset, or verify that this gets compiled down to that
        for (size_t i = old_size; i < new_size; ++i) {
            data_[i] = std::byte{0};
        }
    }

    void MessageBufferFragment::resize_uninitialized(size_t size) {
        if (capacity() < size) {
            throw std::runtime_error("MessageBufferFragment overflow");
        }

        size_ = size;
    }

    void MessageBufferFragment::clear() {
        size_ = 0;
        head_ = false;
        tail_ = false;
    }

    bool MessageBufferFragment::head() const {
        return head_;
    }

    void MessageBufferFragment::set_head(bool head) {
        head_ = head;
    }

    bool MessageBufferFragment::tail() const {
        return tail_;
    }

    void MessageBufferFragment::set_tail(bool tail) {
        tail_ = tail;
    }

}
