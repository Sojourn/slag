#pragma once

#include <memory>
#include <cstddef>

namespace slag {

    class Task;

    class TaskQueue {
    public:
        TaskQueue(size_t minimum_capacity=16);
        TaskQueue(TaskQueue&&) = delete;
        TaskQueue(const TaskQueue&) = delete;

        TaskQueue& operator=(TaskQueue&&) = delete;
        TaskQueue& operator=(const TaskQueue&) = delete;

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] size_t size() const;
        void push_front(Task& task);
        void push_back(Task& task);
        [[nodiscard]] Task* pop_front();
        [[nodiscard]] Task* pop_back();
        [[nodiscard]] Task* peek_front(size_t relative_offset=0);
        [[nodiscard]] Task* peek_back(size_t relative_offset=0);
        void clear();
        void reserve(size_t minimum_capacity);

    private:
        using TaskPointer = Task*;

        std::unique_ptr<TaskPointer[]> data_;
        size_t                         capacity_;
        size_t                         mask_;
        size_t                         head_;
        size_t                         tail_;
    };

}
