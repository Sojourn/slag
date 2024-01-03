#include "task.h"
#include "slag/context.h"
#include <stdexcept>

namespace slag {

    Task::Task(TaskPriority priority)
        : state_{TaskState::WAITING}
        , priority_{priority}
    {
        // We can't automatically subscribe here because
        // the implementation's constructor hasn't run yet
        // to initialize the vtable.
    }

    Event& Task::complete_event() {
        return complete_event_;
    }

    TaskState Task::state() const {
        return state_;
    }

    TaskPriority Task::priority() const {
        return priority_;
    }

    bool Task::is_waiting() const {
        return state_ == TaskState::WAITING;
    }

    bool Task::is_running() const {
        return state_ == TaskState::RUNNING;
    }

    bool Task::is_success() const {
        return state_ == TaskState::SUCCESS;
    }

    bool Task::is_failure() const {
        return state_ == TaskState::FAILURE;
    }

    void Task::set_success(bool success) {
        set_state(success ? TaskState::SUCCESS : TaskState::FAILURE);
    }

    void Task::set_success() {
        set_state(TaskState::SUCCESS);
    }

    void Task::set_failure() {
        set_state(TaskState::FAILURE);
    }

    void Task::set_state(TaskState state) {
        if (!is_valid_transition(state_, state)) {
            assert(false);
            throw std::runtime_error("Invalid task state transition");
        }

        state_ = state;
        if (is_terminal(state)) {
            complete_event_.set();
        }
    }

}
