#include <new>
#include <limits>
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
        // find a power-of-two that is >= minimum_capacity
        size_t capacity = 4; // consumer/producer each reserve 1/4 the capacity for pending updates
        while (capacity < minimum_capacity) {
            capacity *= 2;
        }
        if (capacity > std::numeric_limits<uint32_t>::max()) {
            throw std::runtime_error("SpscQueue capacity overflow");
        }

        slots_.resize(capacity);
    }

    template<typename T>
    inline SpscQueue<T>::~SpscQueue() {
        // lifetime of the queue should be longer than either the producer or consumer
        assert(!producer_);
        assert(!consumer_);

        // drain slots that weren't consumed
        uint32_t head = head_.load();
        uint32_t tail = tail_.load();
        for (uint32_t i = head; i != tail; ++i) {
            slots_[i & (slots_.size() - 1)].destroy();
        }
    }

    template<typename T>
    inline size_t SpscQueue<T>::capacity() const {
        return slots_.size();
    }

    template<typename T>
    template<typename... Args>
    inline void SpscQueue<T>::Slot::create(Args&&... args) {
        new(storage_.data()) T(std::forward<Args>(args)...);
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
        , capacity_{static_cast<uint32_t>(queue.slots_.size())}
        , mask_{capacity_ - 1}
        , pending_insert_count_{0}
        , max_pending_insert_count_{capacity_ / 4}
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
    template<typename... Args>
    inline bool SpscQueueProducer<T>::insert(Args&&... args) {
        size_t item_count = 1;
        size_t slot_count = available_slots(false);
        if (slot_count < item_count) {
            slot_count = available_slots(true);
            if (slot_count < item_count) {
                return false;
            }
        }

        Slot& slot = slots_[cached_tail_ & mask_];
        slot.create(std::forward<Args>(args)...);

        cached_tail_ += item_count;
        pending_insert_count_ += item_count;
        if (max_pending_insert_count_ <= pending_insert_count_) {
            flush();
        }

        return true;
    }

    template<typename T>
    inline bool SpscQueueProducer<T>::insert(std::span<T> items) {
        size_t item_count = items.size();
        size_t slot_count = available_slots(false);
        if (slot_count < item_count) {
            slot_count = available_slots(true);
            if (slot_count < item_count) {
                return false;
            }
        }

        for (size_t i = 0; i < item_count; ++i) {
            Slot& slot = slots_[(cached_tail_ + i) & mask_];
            slot.create(std::move(items[i]));
        }

        cached_tail_ += item_count;
        pending_insert_count_ += item_count;
        if (max_pending_insert_count_ <= pending_insert_count_) {
            flush();
        }

        return true;
    }

    template<typename T>
    inline bool SpscQueueProducer<T>::insert(std::span<const T> items) {
        size_t item_count = items.size();
        size_t slot_count = available_slots(false);
        if (slot_count < item_count) {
            slot_count = available_slots(true);
            if (slot_count < item_count) {
                return false;
            }
        }

        for (size_t i = 0; i < item_count; ++i) {
            Slot& slot = slots_[(cached_tail_ + i) & mask_];
            slot.create(items[i]);
        }

        cached_tail_ += item_count;
        pending_insert_count_ += item_count;
        if (max_pending_insert_count_ <= pending_insert_count_) {
            flush();
        }

        return true;
    }

    template<typename T>
    inline void SpscQueueProducer<T>::flush() {
        if (!pending_insert_count_) {
            return; // fast path; nothing to flush
        }

        tail_.store(cached_tail_);
        pending_insert_count_ = 0;
    }

    template<typename T>
    inline size_t SpscQueueProducer<T>::available_slots(bool synchronize) {
        if (synchronize) {
            cached_head_ = head_.load();
        }

        return capacity_ - (cached_tail_ - cached_head_);
    }

    template<typename T>
    inline SpscQueueConsumer<T>::SpscQueueConsumer(SpscQueue<T>& queue)
        : queue_{queue}
        , slots_{queue.slots_.data()}
        , capacity_{static_cast<uint32_t>(queue.slots_.size())}
        , mask_{capacity_ - 1}
        , pending_remove_count_{0}
        , max_pending_remove_count_{capacity_ / 4}
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
    inline size_t SpscQueueConsumer<T>::poll(std::span<T*> items) {
        size_t item_count = items.size();
        size_t slot_count = available_slots(false);
        if (!slot_count) {
            slot_count = available_slots(true);
        }

        size_t count = std::min(item_count, slot_count);
        for (size_t i = 0; i < count; ++i) {
            Slot& slot = slots_[(cached_head_ + i) & mask_];
            items[i] = &slot.get();
        }
        for (size_t i = count; count < item_count; ++i) {
            items[i] = nullptr;
        }

        return count;
    }

    template<typename T>
    inline void SpscQueueConsumer<T>::remove(size_t count) {
        size_t slot_count = available_slots(false);
        if (count < slot_count) {
            throw std::runtime_error("Invalid count");
        }

        for (size_t i = 0; i < count; ++i) {
            Slot& slot = slots_[(cached_head_ + i) & mask_];
            slot.destroy();
        }

        cached_head_ += count;
        pending_remove_count_ += count;
        if (max_pending_remove_count_ <= pending_remove_count_) {
            flush();
        }
    }

    template<typename T>
    inline void SpscQueueConsumer<T>::flush() {
        if (!pending_remove_count_) {
            return; // fast path; nothing to flush
        }

        head_.store(cached_head_);
        pending_remove_count_ = 0;
    }

    template<typename T>
    inline size_t SpscQueueConsumer<T>::available_slots(bool synchronize) {
        if (synchronize) {
            cached_tail_ = tail_.load();
        }

        return cached_tail_ - cached_head_;
    }

}
