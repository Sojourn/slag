#pragma once

#include <span>
#include <array>
#include <vector>
#include <atomic>
#include <utility>
#include <cstdint>
#include <cstddef>

namespace slag {

    using SpscQueueSequence       = uint32_t;
    using AtomicSpscQueueSequence = std::atomic_uint32_t;

    template<typename U>
    class SpscQueueProducer;

    template<typename U>
    class SpscQueueConsumer;

    template<typename T>
    class SpscQueue {
        template<typename U>
        friend class SpscQueueProducer;

        template<typename U>
        friend class SpscQueueConsumer;

        SpscQueue(SpscQueue&&) = delete;
        SpscQueue(const SpscQueue&) = delete;
        SpscQueue& operator=(SpscQueue&&) = delete;
        SpscQueue& operator=(const SpscQueue&) = delete;

    public:
        explicit SpscQueue(size_t minimum_capacity);
        ~SpscQueue();

        [[nodiscard]] size_t capacity() const;

    private:
        class Slot {
        public:
            template<typename... Args>
            void create(Args&&... args);
            void destroy();

            [[nodiscard]] T& get();

        private:
            alignas(64) std::array<std::byte, sizeof(T)> storage_;
        };

        std::vector<Slot>                              slots_;
        alignas(64) AtomicSpscQueueSequence            head_; // sequence of the next slot to be consumed (front)
        alignas(64) std::atomic<SpscQueueConsumer<T>*> consumer_;
        alignas(64) AtomicSpscQueueSequence            tail_; // sequence of the next slot to be produced (back)
        alignas(64) std::atomic<SpscQueueProducer<T>*> producer_;
    };

    // TODO: make a wrapper for this that can backlog failed insertions
    template<typename T>
    class alignas(64) SpscQueueProducer {

    public:
        SpscQueueProducer();
        explicit SpscQueueProducer(SpscQueue<T>& queue);
        SpscQueueProducer(SpscQueueProducer&& other);
        SpscQueueProducer(const SpscQueueProducer&) = delete;
        ~SpscQueueProducer();

        SpscQueueProducer& operator=(SpscQueueProducer&& rhs);
        SpscQueueProducer& operator=(const SpscQueueProducer&) = delete;

        // Attempt to add a new item to the back of the queue. This may not be immediatly visible
        // to the consumer until flush/reset is called.
        template<typename... Args>
        [[nodiscard]] bool insert(Args&&... args);
        [[nodiscard]] bool insert(std::span<T> items);
        [[nodiscard]] bool insert(std::span<const T> items);

        // This will make insertions visible to the consumer. It will be automatically
        // called after a number of inserts to prevent livelocking.
        SpscQueueSequence flush();

        // Flushes any unsynchronized insertions and detaches this from the queue.
        void reset();

    private:
        [[nodiscard]] size_t available_slots(bool synchronize);

    private:
        using Slot = typename SpscQueue<T>::Slot;

        SpscQueue<T>*                  queue_;
        Slot*                          slots_;
        SpscQueueSequence              capacity_;
        SpscQueueSequence              mask_;
        SpscQueueSequence              pending_insert_count_;
        SpscQueueSequence              max_pending_insert_count_;
        SpscQueueSequence              cached_head_;
        SpscQueueSequence              cached_tail_;
        const AtomicSpscQueueSequence* head_;
        AtomicSpscQueueSequence*       tail_;
    };

    template<typename T>
    class alignas(64) SpscQueueConsumer {
    public:
        SpscQueueConsumer();
        explicit SpscQueueConsumer(SpscQueue<T>& queue);
        SpscQueueConsumer(SpscQueueConsumer&& other);
        SpscQueueConsumer(const SpscQueueConsumer&) = delete;
        ~SpscQueueConsumer();

        SpscQueueConsumer& operator=(SpscQueueConsumer&& rhs);
        SpscQueueConsumer& operator=(const SpscQueueConsumer&) = delete;

        // Polls the queue and sets pointers to items that are ready to be consumed. Multiple calls to
        // poll without interleaved remove calls will yield the same items.
        template<size_t N>
        [[nodiscard]] size_t poll(T* (&items)[N]);

        template<size_t N>
        [[nodiscard]] size_t poll(std::array<T*, N>& items);

        [[nodiscard]] size_t poll(std::span<T*> items);

        // Destroys items returned by pool and advances the consumer. The space occupied by the removed
        // items may not be visible to the producer until flush/reset is called.
        void remove(size_t count);

        // This will make removals visible to the producer. It will be automatically
        // called after a number of removals to prevent livelocking.
        SpscQueueSequence flush();

        // Flushes any unsynchronized removes and detaches this from the queue.
        void reset();

    private:
        [[nodiscard]] size_t available_slots(bool synchronize);

    private:
        using Slot = typename SpscQueue<T>::Slot;

        SpscQueue<T>*                  queue_;
        Slot*                          slots_;
        SpscQueueSequence              capacity_;
        SpscQueueSequence              mask_;
        SpscQueueSequence              pending_remove_count_;
        SpscQueueSequence              max_pending_remove_count_;
        SpscQueueSequence              cached_head_;
        SpscQueueSequence              cached_tail_;
        AtomicSpscQueueSequence*       head_;
        const AtomicSpscQueueSequence* tail_;
    };

}

#include "spsc_queue.hpp"
