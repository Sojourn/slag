#include <cassert>

namespace slag::postal {

    template<typename TaskImpl>
    inline TaskGroup<TaskImpl>::TaskGroup(size_t initial_capacity)
        : task_entry_allocator_{initial_capacity}
        , task_entries_{initial_capacity}
    {
        selector_.insert(executor_.runnable_event());
    }

    template<typename TaskImpl>
    inline TaskGroup<TaskImpl>::~TaskGroup() {
        while (Entry* entry= task_entries_.front()) {
            reap(*entry, false);
        }
    }

    template<typename TaskImpl>
    inline bool TaskGroup<TaskImpl>::is_empty() const {
        return task_entries_.is_empty();
    }

    template<typename TaskImpl>
    inline size_t TaskGroup<TaskImpl>::size() const {
        return task_entries_.size();
    }

    template<typename TaskImpl>
    template<typename... Args>
    inline TaskImpl& TaskGroup<TaskImpl>::spawn(Args&&... args) {
        Entry& entry = task_entry_allocator_.allocate(std::forward<Args>(args)...);
        task_entries_.push_back(entry);

        Event& complete_event = entry.task.complete_event();
        complete_event.set_user_data(&entry);
        selector_.insert(complete_event);
        executor_.insert(entry.task);

        return entry.task;
    }

    template<typename TaskImpl>
    inline Event& TaskGroup<TaskImpl>::runnable_event() {
        return selector_.readable_event();
    }

    template<typename TaskImpl>
    inline void TaskGroup<TaskImpl>::run() {
        Event* event = selector_.select();
        assert(event);

        // We will get scheduled when one of our tasks is runnable or has completed.
        if (void* user_data = event->user_data()) {
            Entry* entry = static_cast<Entry*>(user_data);
            assert(entry->task.is_complete());
            reap(*entry, true);
        }
        else {
            executor_.run();
            selector_.insert(executor_.runnable_event());
        }
    }

    template<typename TaskImpl>
    inline void TaskGroup<TaskImpl>::reap(TaskImpl& task) {
        assert(is_terminal(task.state()));
        assert(task.is_complete());
    }

    template<typename TaskImpl>
    inline void TaskGroup<TaskImpl>::reap(Entry& entry, bool reap_task) {
        if (reap_task) {
            reap(entry.task);
        }

        task_entries_.erase(entry);
        task_entry_allocator_.deallocate(entry);
    }

    template<typename TaskImpl>
    template<typename... Args>
    inline TaskGroup<TaskImpl>::Entry::Entry(Args&&... args)
        : task{std::forward<Args>(args)...}
    {
    }

}