#pragma once

#include "slag/task_queue.h"

namespace slag {

    class Task;

    enum class TaskPriority {
        NORMAL,
        HIGH,
    };

    class Executor {
    public:
        Executor() = default;
        Executor(Executor&&) = delete;
        Executor(const Executor&) = delete;
        ~Executor();

        Executor& operator=(Executor&&) = delete;
        Executor& operator=(const Executor&) = delete;

        [[nodiscard]] bool is_idle() const;
        [[nodiscard]] void run(size_t task_limit=1);

        void schedule(Task& task, TaskPriority priority=TaskPriority::NORMAL);
        void cancel(Task& task);

    private:
        friend class Task;

        struct ScheduledTaskEntry {
            TaskPriority        priority = TaskPriority::NORMAL;
            TaskQueue::Sequence sequence = 0;
        };

        void handle_task_destroyed(Task& task);

    private:
        void garbage_collect();

    private:
        TaskQueue scheduled_tasks_;
    };

}
