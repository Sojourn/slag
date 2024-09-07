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
        if (capacity > std::numeric_limits<SpscQueueSequence>::max()) {
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
        SpscQueueSequence head = head_.load();
        SpscQueueSequence tail = tail_.load();
        for (SpscQueueSequence i = head; i != tail; ++i) {
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
    inline SpscQueueProducer<T>::SpscQueueProducer()
        : queue_{nullptr}
        , slots_{nullptr}
        , capacity_{0}
        , mask_{0}
        , pending_insert_count_{0}
        , max_pending_insert_count_{0}
        , cached_head_{0}
        , cached_tail_{0}
        , head_{nullptr}
        , tail_{nullptr}
    {
    }

    template<typename T>
    inline SpscQueueProducer<T>::SpscQueueProducer(SpscQueue<T>& queue)
        : queue_{&queue}
        , slots_{queue.slots_.data()}
        , capacity_{static_cast<SpscQueueSequence>(queue.slots_.size())}
        , mask_{capacity_ - 1}
        , pending_insert_count_{0}
        , max_pending_insert_count_{capacity_ / 4}
        , cached_head_{queue.head_.load()}
        , cached_tail_{queue.tail_.load()}
        , head_{&queue.head_}
        , tail_{&queue.tail_}
    {
        SpscQueueProducer<T>* expected = nullptr;
        if (!queue_->producer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
            throw std::runtime_error("Too many producers");
        }
    }

    template<typename T>
    inline SpscQueueProducer<T>::SpscQueueProducer(SpscQueueProducer&& other)
        : queue_{other.queue_}
        , slots_{other.slots_}
        , capacity_{other.capacity_}
        , mask_{other.mask_}
        , pending_insert_count_{other.pending_insert_count_}
        , max_pending_insert_count_{other.max_pending_insert_count_}
        , cached_head_{other.cached_head_}
        , cached_tail_{other.cached_head_}
        , head_{other.head_}
        , tail_{other.tail_}
    {
        if (queue_) {
            SpscQueueProducer<T>* expected = &other;
            if (!queue_->producer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
                throw std::runtime_error("Failed to move producer");
            }

            other.queue_                    = nullptr;
            other.slots_                    = nullptr;
            other.capacity_                 = 0;
            other.mask_                     = 0;
            other.pending_insert_count_     = 0;
            other.max_pending_insert_count_ = 0;
            other.cached_head_              = 0;
            other.cached_tail_              = 0;
            other.head_                     = nullptr;
            other.tail_                     = nullptr;
        }
    }

    template<typename T>
    inline SpscQueueProducer<T>::~SpscQueueProducer() {
        reset();
    }

    template<typename T>
    inline SpscQueueProducer<T>& SpscQueueProducer<T>::operator=(SpscQueueProducer&& rhs) {
        if (this != &rhs) {
            reset();

            if (rhs.queue_) {
                std::swap(queue_,                    rhs.queue_);
                std::swap(slots_,                    rhs.slots_);
                std::swap(capacity_,                 rhs.capacity_);
                std::swap(mask_,                     rhs.mask_);
                std::swap(pending_insert_count_,     rhs.pending_insert_count_);
                std::swap(max_pending_insert_count_, rhs.max_pending_insert_count_);
                std::swap(cached_head_,              rhs.cached_head_);
                std::swap(cached_tail_,              rhs.cached_tail_);
                std::swap(head_,                     rhs.head_);
                std::swap(tail_,                     rhs.tail_);

                SpscQueueProducer<T>* expected = &rhs;
                if (!queue_->producer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
                    throw std::runtime_error("Failed to move producer");
                }
            }
        }

        return *this;
    }

    template<typename T>
    template<typename... Args>
    inline bool SpscQueueProducer<T>::insert(Args&&... args) {
        assert(queue_);

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
        assert(queue_);

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
        assert(queue_);

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
    inline SpscQueueSequence SpscQueueProducer<T>::flush() {
        assert(queue_);

        if (pending_insert_count_) {
            tail_->store(cached_tail_);
            pending_insert_count_ = 0;
        }
        else {
            // fast-path; nothing to flush
        }

        return cached_tail_;
    }

    template<typename T>
    inline void SpscQueueProducer<T>::reset() {
        if (!queue_) {
            return;
        }

        // make any pending inserts visible before we clear our state
        flush();

        SpscQueueProducer<T>* expected = this;
        if (!queue_->producer_.compare_exchange_strong(expected, nullptr, std::memory_order_relaxed)) {
            assert(false); // internal error
        }

        queue_                    = nullptr;
        slots_                    = nullptr;
        capacity_                 = 0;
        mask_                     = 0;
        pending_insert_count_     = 0;
        max_pending_insert_count_ = 0;
        cached_head_              = 0;
        cached_tail_              = 0;
        head_                     = nullptr;
        tail_                     = nullptr;
    }

    template<typename T>
    inline size_t SpscQueueProducer<T>::available_slots(bool synchronize) {
        assert(queue_);

        if (synchronize) {
            cached_head_ = head_->load();
        }

        return capacity_ - (cached_tail_ - cached_head_);
    }

    template<typename T>
    inline SpscQueueConsumer<T>::SpscQueueConsumer()
        : queue_{nullptr}
        , slots_{nullptr}
        , capacity_{0}
        , mask_{0}
        , pending_remove_count_{0}
        , max_pending_remove_count_{0}
        , cached_head_{0}
        , cached_tail_{0}
        , head_{nullptr}
        , tail_{nullptr}
    {
    }    

    template<typename T>
    inline SpscQueueConsumer<T>::SpscQueueConsumer(SpscQueue<T>& queue)
        : queue_{&queue}
        , slots_{queue.slots_.data()}
        , capacity_{static_cast<SpscQueueSequence>(queue.slots_.size())}
        , mask_{capacity_ - 1}
        , pending_remove_count_{0}
        , max_pending_remove_count_{capacity_ / 4}
        , cached_head_{queue.head_.load()}
        , cached_tail_{queue.tail_.load()}
        , head_{&queue.head_}
        , tail_{&queue.tail_}
    {
        SpscQueueConsumer<T>* expected = nullptr;
        if (!queue_->consumer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
            throw std::runtime_error("Too many consumers");
        }
    }

    template<typename T>
    inline SpscQueueConsumer<T>::SpscQueueConsumer(SpscQueueConsumer&& other)
        : queue_{other.queue_}
        , slots_{other.slots_}
        , capacity_{other.capacity_}
        , mask_{other.mask_}
        , pending_remove_count_{other.pending_remove_count_}
        , max_pending_remove_count_{other.max_pending_remove_count_}
        , cached_head_{other.cached_head_}
        , cached_tail_{other.cached_head_}
        , head_{other.head_}
        , tail_{other.tail_}
    {
        if (queue_) {
            SpscQueueConsumer<T>* expected = &other;
            if (!queue_->consumer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
                throw std::runtime_error("Failed to move consumer");
            }

            other.queue_                    = nullptr;
            other.slots_                    = nullptr;
            other.capacity_                 = 0;
            other.mask_                     = 0;
            other.pending_remove_count_     = 0;
            other.max_pending_remove_count_ = 0;
            other.cached_head_              = 0;
            other.cached_tail_              = 0;
            other.head_                     = nullptr;
            other.tail_                     = nullptr;
        }
    }

    template<typename T>
    inline SpscQueueConsumer<T>::~SpscQueueConsumer() {
        reset();
    }

    template<typename T>
    inline SpscQueueConsumer<T>& SpscQueueConsumer<T>::operator=(SpscQueueConsumer&& rhs) {
        if (this != &rhs) {
            reset();

            if (rhs.queue_) {
                std::swap(queue_,                    rhs.queue_);
                std::swap(slots_,                    rhs.slots_);
                std::swap(capacity_,                 rhs.capacity_);
                std::swap(mask_,                     rhs.mask_);
                std::swap(pending_remove_count_,     rhs.pending_remove_count_);
                std::swap(max_pending_remove_count_, rhs.max_pending_remove_count_);
                std::swap(cached_head_,              rhs.cached_head_);
                std::swap(cached_tail_,              rhs.cached_tail_);
                std::swap(head_,                     rhs.head_);
                std::swap(tail_,                     rhs.tail_);

                SpscQueueConsumer<T>* expected = &rhs;
                if (!queue_->consumer_.compare_exchange_strong(expected, this, std::memory_order_relaxed)) {
                    throw std::runtime_error("Failed to move consumer");
                }
            }
        }

        return *this;
    }

    template<typename T>
    template<size_t N>
    inline size_t SpscQueueConsumer<T>::poll(T* (&items)[N]) {
        return poll(std::span{items});
    }

    template<typename T>
    template<size_t N>
    inline size_t SpscQueueConsumer<T>::poll(std::array<T*, N>& items) {
        return poll(std::span{items.data(), N});
    }

    template<typename T>
    inline size_t SpscQueueConsumer<T>::poll(std::span<T*> items) {
        assert(queue_);

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

        return count;
    }

    template<typename T>
    inline void SpscQueueConsumer<T>::remove(size_t count) {
        assert(queue_);

        size_t slot_count = available_slots(false);
        if (slot_count < count) {
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
    inline SpscQueueSequence SpscQueueConsumer<T>::flush() {
        assert(queue_);

        if (pending_remove_count_) {
            head_->store(cached_head_);
            pending_remove_count_ = 0;
        }
        else {
            // nothing to flush
        }

        return cached_head_;
    }


    template<typename T>
    inline void SpscQueueConsumer<T>::reset() {
        if (!queue_) {
            return;
        }

        flush();

        SpscQueueConsumer<T>* expected = this;
        if (!queue_->consumer_.compare_exchange_strong(expected, nullptr, std::memory_order_relaxed)) {
            assert(false); // internal error
        }

        queue_                    = nullptr;
        slots_                    = nullptr;
        capacity_                 = 0;
        mask_                     = 0;
        pending_remove_count_     = 0;
        max_pending_remove_count_ = 0;
        cached_head_              = 0;
        cached_tail_              = 0;
        head_                     = nullptr;
        tail_                     = nullptr;
    }

    template<typename T>
    inline size_t SpscQueueConsumer<T>::available_slots(bool synchronize) {
        assert(queue_);

        if (synchronize) {
            cached_tail_ = tail_->load();
        }

        return cached_tail_ - cached_head_;
    }

}
