#pragma once

#include "slag/queue.h"
#include "slag/postal/event.h"
#include "slag/postal/pollable.h"

namespace slag::postal {

    template<typename T>
    class PollableAdapter<Queue<T>> : public Pollable<PollableType::READABLE> {
    public:
        Event& readable_event() {
            return readable_event_;
        }

        bool is_empty() const {
            return queue_.is_empty();
        }

        size_t size() const {
            return queue_.size();
        }

        size_t capacity() const {
            return queue_.capacity();
        }

        T& front() {
            return queue_.front();
        }

        const T& front() const {
            return queue_.front();
        }

        T& back() {
            return queue_.back();
        }

        const T& back() const {
            return queue_.back();
        }

        T& operator[](size_t index) {
            return queue_[index];
        }

        const T& operator[](size_t index) const {
            return queue_[index];
        }

        template<typename... Args>
        void emplace_back(Args&&... args) {
            queue_.emplace_back(std::forward<Args>(args)...);
            update_readiness();
        }

        void push_back(T value) {
            queue_.push_back(std::move(value));
            update_readiness();
        }

        std::optional<T> pop_front() {
            auto result = queue_.pop_front();
            update_readiness();
            return result;
        }

        bool consume_front() {
            bool result = queue_.consume_front();
            update_readiness();
            return result;
        }

        void clear() {
            queue_.clear();
            update_readiness();
        }

        void reserve(size_t minimum_capacity) {
            queue_.reserve(minimum_capacity);
        }

    private:
        void update_readiness() {
            readable_event_.set(
                !queue_.is_empty()
            );
        }

    private:
        Queue<T> queue_;
        Event    readable_event_;
    };

    template<typename T>
    using PollableQueue = PollableAdapter<Queue<T>>;

}
