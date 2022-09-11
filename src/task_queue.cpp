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
    : head_{0}
    , tail_{0}
    , mask_{0}
    , capacity_{0}
    , tombstones_{0}
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

size_t slag::TaskQueue::tombstones() const {
    return tombstones_;
}

slag::TaskQueue::Sequence slag::TaskQueue::push_front(Task& task) {
    if (size() == capacity_) {
        reserve(capacity_ * 2); // grow
    }

    Sequence sequence = --head_;
    get_slot(sequence) = &task;
    return sequence;
}

slag::TaskQueue::Sequence slag::TaskQueue::push_back(Task& task) {
    if (size() == capacity_) {
        reserve(capacity_ * 2); // grow
    }

    Sequence sequence = tail_++;
    get_slot(sequence) = &task;
    return sequence;
}

slag::Task* slag::TaskQueue::pop_front() {
    if (is_empty()) {
        return nullptr;
    }

    Slot& slot = get_slot(head_++);
    Task* task = slot; // copy
    if (slot) {
        slot = nullptr;
    }
    else {
        assert(tombstones_ > 0);
        --tombstones_; // popped a tombstone
    }

    return task; 
}

slag::Task* slag::TaskQueue::pop_back() {
    if (is_empty()) {
        return nullptr;
    }

    Slot& slot = get_slot(--tail_);
    Task* task = slot; // copy
    if (slot) {
        slot = nullptr;
    }
    else {
        assert(tombstones_ > 0);
        --tombstones_; // popped a tombstone
    }

    return task;
}

slag::Task* slag::TaskQueue::peek_front(size_t relative_offset) {
    if (size() <= relative_offset) {
        return nullptr;
    }

    return get_slot(head_ + relative_offset);
}

slag::Task* slag::TaskQueue::peek_back(size_t relative_offset) {
    if (size() <= relative_offset) {
        return nullptr;
    }

    return get_slot(tail_ - relative_offset - 1);
}

void slag::TaskQueue::erase(Sequence sequence) {
    Slot& slot = get_slot(sequence);
    if (slot) {
        slot = nullptr;
        ++tombstones_;
        assert(tombstones_ <= size());
    }
}

void slag::TaskQueue::clear() {
    for (Sequence sequence = head_; sequence != tail_; ++sequence) {
        get_slot(sequence) = nullptr;
    }

    head_       = 0;
    tail_       = 0;
    tombstones_ = 0;
}

void slag::TaskQueue::reserve(size_t minimum_capacity) {
    if (capacity_ >= minimum_capacity) {
        return;
    }

    size_t capacity = make_capacity(minimum_capacity);
    size_t mask     = capacity - 1;

    // copy everything (including tombstones) into a new, larger array
    std::unique_ptr<Slot[]> slots{new Slot[capacity]};
    for (Sequence sequence = head_; sequence != tail_; ++sequence) {
        get_slot(slots, mask, sequence) = get_slot(sequence);
    }

    slots_    = std::move(slots);
    capacity_ = capacity;
    mask_     = mask;
    // head_ and tail_ are preserved to avoid invalidating sequences
}

slag::TaskQueue::Slot& slag::TaskQueue::get_slot(Sequence sequence) {
    return get_slot(slots_, mask_, sequence);
}

slag::TaskQueue::Slot& slag::TaskQueue::get_slot(std::unique_ptr<Slot[]>& slots, size_t mask, Sequence sequence) {
    return slots[sequence & mask];
}
