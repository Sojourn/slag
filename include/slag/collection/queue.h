#pragma once

#include <array>
#include <optional>
#include <memory>
#include <cstddef>
#include <cstdint>

namespace slag {

    template<typename T>
    class Queue {
    public:
        Queue();
        ~Queue();

        Queue(Queue&& other);
        Queue(const Queue& other) = delete;
        Queue& operator=(Queue&& rhs);
        Queue& operator=(const Queue& rhs) = delete;

        bool is_empty() const;
        size_t size() const;
        size_t capacity() const;

        T& front();
        const T& front() const;
        T& back();
        const T& back() const;

        // the front element has an index of zero
        T& operator[](size_t index);
        const T& operator[](size_t index) const;

        template<typename... Args>
        void emplace_back(Args&&... args);
        void push_back(T value);
        std::optional<T> pop_front();
        bool consume_front(size_t count = 1);

        void clear();
        void reserve(size_t minimum_capacity);

    private:
        class Slot {
        public:
            template<typename... Args>
            void create(Args&&... args);
            void destroy();

            T& get();
            const T& get() const;

        private:
            alignas(alignof(T)) std::array<std::byte, sizeof(T)> storage_;
        };

    private:
        size_t                  capacity_;
        size_t                  mask_;
        size_t                  head_;
        size_t                  tail_;
        std::unique_ptr<Slot[]> slots_;
    };

}

#include "queue.hpp"
