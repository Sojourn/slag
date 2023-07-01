#include <new>
#include <stdexcept>
#include <cassert>

namespace slag {

    template<typename T>
    inline SpscQueue<T>::SpscQueue(size_t minimum_capacity)
        : head_{0}
        , consumer_{nullptr}
        , tail_{0}
        , producer_{nullptr}
    {
        size_t capacity = 1;
        while (capacity < minimum_capacity) {
            capacity *= 2;
        }

        slots_.resize(capacity);
    }

    template<typename T>
    inline SpscQueue<T>::~SpscQueue() {
        uint64_t head = head_.load();
        uint64_t tail = tail_.load();

        for (uint64_t i = head; i != tail; ++i) {
            slots_[i & (slots_.size() - 1)].destroy();
        }

        assert(!producer_);
        assert(!consumer_);
    }

    template<typename T>
    inline size_t SpscQueue<T>::capacity() const {
        return slots_.size();
    }

    template<typename T>
    template<typename... Args>
    inline void SpscQueue<T>::Slot::create(Args&&... args) {
        new(storage_.data()) T{std::forward<Args>(args)...};
    }

    template<typename T>
    inline void SpscQueue<T>::Slot::destroy() {
        get().~T();
    }

    template<typename T>
    inline T& SpscQueue<T>::Slot::get() {
        return *reinterpret_cast<T*>(storage_.data());
    }

    template<typename T>
    inline SpscQueueProducer<T>::SpscQueueProducer(SpscQueue<T>& queue)
        : queue_{queue}
        , slots_{queue.slots_.data()}
        , capacity_{queue.slots_.size()}
        , mask_{queue.slots_.size() - 1}
        , cached_head_{queue.head_}
        , cached_tail_{queue.tail_}
        , head_{queue.head_}
        , tail_{queue.tail_}
    {
        SpscQueueProducer<T>* expected = nullptr;
        if (!queue_.producer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
            throw std::runtime_error("Too many producers");
        }
    }

    template<typename T>
    inline SpscQueueProducer<T>::~SpscQueueProducer() {
        SpscQueueProducer<T>* expected = this;
        if (!queue_.producer_.compare_exchange_strong(expected, nullptr, std::memory_order_relaxed)) {
            assert(false); // internal error
        }
    }

    template<typename T>
    inline bool SpscQueueProducer<T>::produce(T item) {
        return produce(std::span{&item, 1}) > 0;
    }

    template<typename T>
    inline size_t SpscQueueProducer<T>::produce(std::span<T> items) {
        size_t item_count = items.size();
        size_t slot_count = available_slots(false);

        // reduces variance vs. slot_count < 0 when the queue is nearly full
        if (slot_count < item_count) {
            slot_count = available_slots(true);
        }

        slot_count = std::min(slot_count, item_count);
        for (size_t i = 0; i < slot_count; ++i) {
            Slot& slot = slots_[(cached_tail_ + i) & mask_];
            slot.create(std::move(items[i]));
        }

        cached_tail_ += slot_count;
        tail_.store(cached_tail_);
        return slot_count;
    }

    template<typename T>
    inline size_t SpscQueueProducer<T>::available_slots(bool refresh) {
        if (refresh) {
            cached_head_ = head_.load();
        }

        return capacity_ - (cached_tail_ - cached_head_);
    }

    template<typename T>
    inline SpscQueueConsumer<T>::SpscQueueConsumer(SpscQueue<T>& queue)
        : queue_{queue}
        , slots_{queue.slots_.data()}
        , capacity_{queue.slots_.size()}
        , mask_{queue.slots_.size() - 1}
        , cached_head_{queue.head_}
        , cached_tail_{queue.tail_}
        , head_{queue.head_}
        , tail_{queue.tail_}
    {
        SpscQueueConsumer<T>* expected = nullptr;
        if (!queue_.consumer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
            throw std::runtime_error("Too many consumers");
        }
    }

    template<typename T>
    inline SpscQueueConsumer<T>::~SpscQueueConsumer() {
        SpscQueueConsumer<T>* expected = this;
        if (!queue_.consumer_.compare_exchange_strong(expected, nullptr, std::memory_order_relaxed)) {
            assert(false); // internal error
        }
    }

    template<typename T>
    inline bool SpscQueueConsumer<T>::consume(T& item) {
        return consume(std::span{&item, 1}) > 0;
    }

    template<typename T>
    inline size_t SpscQueueConsumer<T>::consume(std::span<T> items) {
        size_t item_count = items.size();
        size_t slot_count = available_slots(false);

        // reduces variance vs. slot_count < 0 when the queue is nearly empty
        if (slot_count < item_count) {
            slot_count = available_slots(true);
        }

        slot_count = std::min(slot_count, item_count);
        for (size_t i = 0; i < slot_count; ++i) {
            Slot& slot = slots_[(cached_head_ + i) & mask_];
            items[i] = std::move(slot.get());
            slot.destroy();
        }

        cached_head_ += slot_count;
        head_.store(cached_head_);
        return slot_count;
    }

    template<typename T>
    inline size_t SpscQueueConsumer<T>::available_slots(bool refresh) {
        if (refresh) {
            cached_tail_ = tail_.load();
        }

        return cached_tail_ - cached_head_;
    }

}
