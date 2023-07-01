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
        alignas(64) std::atomic_uint_fast64_t          head_; // sequence of the next slot to be consumed (front)
        alignas(64) std::atomic<SpscQueueConsumer<T>*> consumer_;
        alignas(64) std::atomic_uint_fast64_t          tail_; // sequence of the next slot to be produced (back)
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

        [[nodiscard]] bool produce(T item);
        [[nodiscard]] size_t produce(std::span<T> items);

    private:
        [[nodiscard]] size_t available_slots(bool refresh);

    private:
        using Slot = typename SpscQueue<T>::Slot;

        SpscQueue<T>&                    queue_;
        Slot*                            slots_;
        uint64_t                         capacity_;
        uint64_t                         mask_;
        uint64_t                         cached_head_;
        uint64_t                         cached_tail_;
        const std::atomic_uint_fast64_t& head_;
        std::atomic_uint_fast64_t&       tail_;
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

        [[nodiscard]] bool consume(T& item);
        [[nodiscard]] size_t consume(std::span<T> items);

    private:
        [[nodiscard]] size_t available_slots(bool refresh);

    private:
        using Slot = typename SpscQueue<T>::Slot;

        SpscQueue<T>&                    queue_;
        Slot*                            slots_;
        uint64_t                         capacity_;
        uint64_t                         mask_;
        uint64_t                         cached_head_;
        uint64_t                         cached_tail_;
        std::atomic_uint_fast64_t&       head_;
        const std::atomic_uint_fast64_t& tail_;
    };

}

#include "spsc_queue.hpp"
