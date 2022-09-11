#include "slag/executor.h"
#include "slag/task.h"
#include <stdexcept>
#include <cassert>

slag::Executor::~Executor() {
    assert(is_idle());
}

bool slag::Executor::is_idle() const {
    return scheduled_tasks_.is_empty();
}

void slag::Executor::run(size_t task_limit) {
    while (!is_idle() && task_limit) {
        Task* task = scheduled_tasks_.pop_front();
        if (!task) {
            continue; // tombstone, skip
        }

        assert(task->is_scheduled());
        task->scheduled_task_entry_.reset();
        task->run();

        task_limit -= 1;
    }

    garbage_collect();
}

void slag::Executor::schedule(Task& task, TaskPriority priority) {
    if (&task.executor_ != this) {
        throw std::runtime_error("Task does not belong to this executor");
    }
    if (task.is_scheduled()) {
        if (static_cast<int>(priority) < static_cast<int>(task.scheduled_task_entry_->priority)) {
            return; // already scheduled with a higher priority
        }

        cancel(task);
    }

    TaskQueue::Sequence sequence = 0;
    switch (priority) {
        case TaskPriority::NORMAL: {
            sequence = scheduled_tasks_.push_back(task);
            break;
        }
        case TaskPriority::HIGH: {
            sequence = scheduled_tasks_.push_front(task);
            break;
        }
    }

    task.scheduled_task_entry_ = ScheduledTaskEntry {
        .priority = priority,
        .sequence = sequence,
    };
}

void slag::Executor::cancel(Task& task) {
    if (&task.executor_ != this) {
        throw std::runtime_error("Task does not belong to this executor");
    }
    if (!task.is_scheduled()) {
        return;
    }

    scheduled_tasks_.erase(task.scheduled_task_entry_->sequence);
    task.scheduled_task_entry_.reset();
}

void slag::Executor::garbage_collect() {
    // TODO: compact the scheduled_tasks_ queue if there are too many tombstones
}
