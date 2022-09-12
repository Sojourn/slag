#pragma once

#include <memory>
#include <cstddef>

namespace slag {

    class Task;

    class TaskQueue {
    public:
        using Sequence = size_t;

        TaskQueue(size_t minimum_capacity=16);
        TaskQueue(TaskQueue&&) = delete;
        TaskQueue(const TaskQueue&) = delete;

        TaskQueue& operator=(TaskQueue&&) = delete;
        TaskQueue& operator=(const TaskQueue&) = delete;

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t capacity() const;
        [[nodiscard]] size_t tombstones() const;
        Sequence push_front(Task& task);
        Sequence push_back(Task& task);
        [[nodiscard]] Task* pop_front();
        [[nodiscard]] Task* pop_back();
        [[nodiscard]] Task* peek_front(size_t relative_offset=0);
        [[nodiscard]] Task* peek_back(size_t relative_offset=0);
        void erase(Sequence sequence); // replaces Task w/ a tombstone
        void clear();
        void reserve(size_t minimum_capacity);

    private:
        using Slot = Task*;

        Slot& get_slot(Sequence sequence);
        static Slot& get_slot(std::unique_ptr<Slot[]>& slots, size_t mask, Sequence sequence);

    private:
        std::unique_ptr<Slot[]> slots_;
        Sequence                head_;
        Sequence                tail_;
        size_t                  mask_;
        size_t                  capacity_;
        size_t                  tombstones_;
    };

}
