#pragma once

#include <string_view>
#include "event.h"
#include "pollable.h"

namespace slag {

    enum class TaskState : uint8_t {
        WAITING, // The task is waiting on an event, or to be scheduled.
        RUNNING, // The task is actively running.
        SUCCESS, // The task has completed succesfully.
        FAILURE, // The task has completed with an unspecified error.
    };

    enum class TaskPriority : uint8_t {
        SAME, // Inherit priority.
        HIGH, // Interleave with servicing the event loop.
        IDLE, // Only run if there are no high-priority tasks runnable.
    };

    class Task
        : public Pollable<PollableType::RUNNABLE>
        , public Pollable<PollableType::COMPLETE>
    {
        // The Executor needs the address of this to be fixed.
        Task(Task&&) = delete;
        Task(const Task&) = delete;
        Task& operator=(Task&&) = delete;
        Task& operator=(const Task&) = delete;

    public:
        explicit Task(TaskPriority priority = TaskPriority::SAME);
        virtual ~Task() = default;

        TaskState state() const;
        TaskPriority priority() const;

        bool is_waiting() const;
        bool is_running() const;
        bool is_success() const;
        bool is_failure() const;

        using Pollable<PollableType::RUNNABLE>::is_runnable;
        using Pollable<PollableType::COMPLETE>::is_complete;

        // This will become set when the task has completed (success/failure).
        Event& complete_event() override final;

        // This should execute a small, but meaningful amount of work. It will
        // be periodically called by an executor when the task indicates
        // that it is runnable (the runnable event is set).
        //
        // Propagating exceptions out of this will cause the task to fail, even
        // if they previously succeeded.
        //
        virtual void run() = 0;

    protected:
        void set_success(bool success);
        void set_success();
        void set_failure();

    private:
        friend class Executor;

        void set_state(TaskState state);

    private:
        TaskState    state_;
        TaskPriority priority_;
        Event        complete_event_;
    };

    constexpr std::string_view to_string(TaskState state) {
        using namespace std::string_view_literals;

        switch (state) {
            case TaskState::WAITING: return "WAITING"sv;
            case TaskState::RUNNING: return "RUNNING"sv;
            case TaskState::SUCCESS: return "SUCCESS"sv;
            case TaskState::FAILURE: return "FAILURE"sv;
        }
    }

    constexpr bool is_terminal(TaskState state) {
        return (state == TaskState::SUCCESS) || (state == TaskState::FAILURE);
    }

    constexpr bool is_valid_transition(TaskState old_state, TaskState new_state) {
        if (is_terminal(old_state)) {
            return false; // Cannot transition out of a terminal state.
        }

        return old_state != new_state; // Self-transitions are unexpected.
    }

}
