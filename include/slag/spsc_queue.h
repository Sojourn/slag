#pragma once

#include <span>
#include <array>
#include <atomic>
#include <utility>
#include <cstdint>
#include <cstddef>

namespace slag {

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
        alignas(64) std::atomic_uint32_t               head_; // sequence of the next slot to be consumed (front)
        alignas(64) std::atomic<SpscQueueConsumer<T>*> consumer_;
        alignas(64) std::atomic_uint32_t               tail_; // sequence of the next slot to be produced (back)
        alignas(64) std::atomic<SpscQueueProducer<T>*> producer_;
    };

    template<typename T>
    class alignas(64) SpscQueueProducer {
        SpscQueueProducer(SpscQueueProducer&&) = delete;
        SpscQueueProducer(const SpscQueueProducer&) = delete;
        SpscQueueProducer& operator=(SpscQueueProducer&&) = delete;
        SpscQueueProducer& operator=(const SpscQueueProducer&) = delete;

    public:
        explicit SpscQueueProducer(SpscQueue<T>& queue);
        ~SpscQueueProducer();

        template<typename... Args>
        [[nodiscard]] bool insert(Args&&... args);
        [[nodiscard]] bool insert(std::span<T> items);
        [[nodiscard]] bool insert(std::span<const T> items);

        // This will make insertions visible to the consumer. It will be automatically
        // called after a number of inserts to prevent livelocking.
        void flush();

    private:
        [[nodiscard]] size_t available_slots(bool synchronize);

    private:
        using Slot = typename SpscQueue<T>::Slot;

        SpscQueue<T>&                    queue_;
        Slot*                            slots_;
        uint32_t                         capacity_;
        uint32_t                         mask_;
        uint32_t                         pending_insert_count_;
        uint32_t                         max_pending_insert_count_;
        uint32_t                         cached_head_;
        uint32_t                         cached_tail_;
        const std::atomic_uint32_t&      head_;
        std::atomic_uint32_t&            tail_;
    };

    template<typename T>
    class alignas(64) SpscQueueConsumer {
        SpscQueueConsumer(SpscQueueConsumer&&) = delete;
        SpscQueueConsumer(const SpscQueueConsumer&) = delete;
        SpscQueueConsumer& operator=(SpscQueueConsumer&&) = delete;
        SpscQueueConsumer& operator=(const SpscQueueConsumer&) = delete;

    public:
        explicit SpscQueueConsumer(SpscQueue<T>& queue);
        ~SpscQueueConsumer();

        [[nodiscard]] size_t poll(std::span<T*> items);

        // Destroys items returned by pool and advances the consumer.
        void remove(size_t count);

        // This will make removals visible to the producer. It will be automatically
        // called after a number of removals to prevent livelocking.
        void flush();

    private:
        [[nodiscard]] size_t available_slots(bool synchronize);

    private:
        using Slot = typename SpscQueue<T>::Slot;

        SpscQueue<T>&                    queue_;
        Slot*                            slots_;
        uint32_t                         capacity_;
        uint32_t                         mask_;
        uint32_t                         pending_remove_count_;
        uint32_t                         max_pending_remove_count_;
        uint32_t                         cached_head_;
        uint32_t                         cached_tail_;
        std::atomic_uint32_t&            head_;
        const std::atomic_uint32_t&      tail_;
    };

}

#include "spsc_queue.hpp"
