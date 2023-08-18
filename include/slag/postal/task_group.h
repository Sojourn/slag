#pragma once

#include "slag/pool_allocator.h"
#include "slag/intrusive_queue.h"
#include "slag/postal/task.h"

namespace slag::postal {

    // Manages the lifetime and execution of a homogeneous collection of tasks.
    // All tasks are destroyed when this goes out of scope, completed or not.
    template<typename TaskImpl>
    class TaskGroup : public Task {
    public:
        explicit TaskGroup(size_t initial_capacity = 0);
        ~TaskGroup();

        bool is_empty() const;
        size_t size() const;

        template<typename... Args>
        TaskImpl& spawn(Args&&... args);

    public:
        Event& runnable_event() override;

        void run() override;

    private:
        struct Entry;

        // Override this if you want to access a task after it has completed.
        // NOTE: This is _not_ invoked when we are leaving scope.
        virtual void reap(TaskImpl& task);

        void reap(Entry& entry, bool reap_task = true);

    private:
        struct Entry {
            IntrusiveQueueNode node;
            TaskImpl           task;

            template<typename... Args>
            Entry(Args&&... args);
        };

        Selector                            selector_;
        Executor                            executor_;
        PoolAllocator<Entry>                task_entry_allocator_;
        IntrusiveQueue<Entry, &Entry::node> task_entries_;
    };

}

#include "slag/postal/task_group.hpp"
