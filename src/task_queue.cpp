#include "slag/task_queue.h"
#include <cassert>

static size_t make_capacity(size_t minimum_capacity) {
    // iterate over powers of two until we find one >= minimum_capacity
    size_t capacity = 1;
    while (capacity < minimum_capacity) {
        capacity *= 2;
    }

    return capacity;
}

slag::TaskQueue::TaskQueue(size_t minimum_capacity)
    : capacity_{0}
    , mask_{0}
    , head_{0}
    , tail_{0}
{
    reserve(minimum_capacity);
}

bool slag::TaskQueue::is_empty() const {
    return head_ == tail_;
}

size_t slag::TaskQueue::size() const {
    return tail_ - head_;
}

size_t slag::TaskQueue::capacity() const {
    return capacity_;
}

void slag::TaskQueue::push_front(Task& task) {
    if ((head_ - tail_) == capacity_) {
        reserve(capacity_ * 2);
    }

    data_[(--head_) & mask_] = &task;
}

void slag::TaskQueue::push_back(Task& task) {
    if ((head_ - tail_) == capacity_) {
        reserve(capacity_ * 2);
    }

    data_[(tail_++) & mask_] = &task;
}

slag::Task* slag::TaskQueue::pop_front() {
    if (is_empty()) {
        return nullptr;
    }

    return data_[(head_++) & mask_];
}

slag::Task* slag::TaskQueue::pop_back() {
    if (is_empty()) {
        return nullptr;
    }

    return data_[(--tail_) & mask_];
}

slag::Task* slag::TaskQueue::peek_front(size_t relative_offset) {
    if (size() <= relative_offset) {
        return nullptr;
    }

    return data_[(head_ + relative_offset) & mask_];
}

slag::Task* slag::TaskQueue::peek_back(size_t relative_offset) {
    if (size() <= relative_offset) {
        return nullptr;
    }

    return data_[(tail_ - relative_offset - 1) & mask_];
}

void slag::TaskQueue::clear() {
    head_ = 0;
    tail_ = 0;
}

void slag::TaskQueue::reserve(size_t minimum_capacity) {
    if (capacity_ >= minimum_capacity) {
        return;
    }

    size_t capacity = make_capacity(minimum_capacity);
    size_t mask     = capacity - 1;
    size_t size     = head_ - tail_;

    // copy data into a new, larger array
    std::unique_ptr<TaskPointer[]> data{new TaskPointer[capacity]};
    if (data_) {
        for (size_t i = 0; i < size; ++i) {
            data_[i] = data[(head_ + i) & mask_];
        }
    }

    data_     = std::move(data);
    capacity_ = capacity;
    mask_     = mask;
    head_     = 0;
    tail_     = size;
}
