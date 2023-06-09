#include "slag/message_record_fragment.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    bool MessageRecordFragment::is_full() const {
        return size_ == MESSAGE_FRAGMENT_CAPACITY;
    }

    bool MessageRecordFragment::is_empty() const {
        return size_ == 0;
    }

    size_t MessageRecordFragment::size() const {
        return size_;
    }

    size_t MessageRecordFragment::capacity() const {
        return MESSAGE_FRAGMENT_CAPACITY;
    }

    std::span<uint64_t> MessageRecordFragment::data() {
        return std::span{data_, size_};
    }

    std::span<const uint64_t> MessageRecordFragment::data() const {
        return std::span{data_, size_};
    }

    uint64_t& MessageRecordFragment::operator[](size_t index) {
        assert(index < size());

        return data_[index];
    }

    const uint64_t& MessageRecordFragment::operator[](size_t index) const {
        assert(index < size());

        return data_[index];
    }

    void MessageRecordFragment::push_back(uint64_t value) {
        if (is_full()) {
            throw std::runtime_error("MessageRecordFragment overflow");
        }

        data_[size_++] = value;
    }

    void MessageRecordFragment::resize(size_t size) {
        if (capacity() < size) {
            throw std::runtime_error("MessageRecordFragment overflow");
        }

        size_t old_size = size_;
        size_t new_size = size;

        resize_uninitialized(new_size);

        // TODO: memset, or verify that this gets compiled down to that
        for (size_t i = old_size; i < new_size; ++i) {
            data_[i] = 0;
        }
    }

    void MessageRecordFragment::resize_uninitialized(size_t size) {
        if (capacity() < size) {
            throw std::runtime_error("MessageRecordFragment overflow");
        }

        size_ = size;
    }

    void MessageRecordFragment::clear() {
        size_ = 0;
        head_ = false;
        tail_ = false;
    }

    bool MessageRecordFragment::head() const {
        return head_;
    }

    void MessageRecordFragment::set_head(bool head) {
        head_ = head;
    }

    bool MessageRecordFragment::tail() const {
        return tail_;
    }

    void MessageRecordFragment::set_tail(bool tail) {
        tail_ = tail;
    }

}
