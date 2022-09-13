#include "slag/executor.h"
#include "slag/task.h"
#include "slag/event_loop.h"
#include <stdexcept>
#include <cassert>

slag::Executor::~Executor() {
    garbage_collect();
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

        // prefetch the next task
        // if (Task* next_task = scheduled_tasks_.peek_front(1)) {
        //     _mm_prefetch(reinterpret_cast<char*>(next_task), _MM_HINT_T0);
        // }

        assert(task->is_scheduled());
        task->scheduled_task_entry_.reset();
        task->run();

        task_limit -= 1;
    }
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

    // compact the scheduled tasks queue if there are a lot of tombstones, and they
    // make up a significant fraction (25%+) of the in-use queue slots.
    bool do_garbage_collection = true;
    do_garbage_collection &= scheduled_tasks_.tombstones() > 128;
    do_garbage_collection &= (scheduled_tasks_.size() / 4) <= scheduled_tasks_.tombstones();
    if (do_garbage_collection) {
        garbage_collect();
    }
}

void slag::Executor::garbage_collect() {
    size_t size = scheduled_tasks_.size();
    for (size_t i = 0; i < size; ++i) {
        if (Task* task = scheduled_tasks_.pop_front()) {
            task->scheduled_task_entry_->sequence = scheduled_tasks_.push_back(*task);
        }
    }

    assert(scheduled_tasks_.size() <= size);
    assert(scheduled_tasks_.tombstones() == 0);
}

slag::Executor& slag::local_executor() {
    return local_event_loop().executor();
}
