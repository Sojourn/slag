#include "slag/postal/task.h"
#include <stdexcept>

namespace slag::postal {

    constexpr bool is_valid_transition(TaskState old_state, TaskState new_state) {
        switch (old_state) {
            case TaskState::WAITING: {
                // Can only transition into a running state.
                return new_state == TaskState::RUNNING;
            }
            case TaskState::RUNNING: {
                // Anything other than a self-transition is fine.
                return new_state != TaskState::RUNNING;
            }
            default: {
                // Already in a terminal state, which cannot self-transition.
                return false;
            }
        }
    }

    Task::Task()
        : state_{TaskState::WAITING}
    {
    }

    Event& Task::complete_event() {
        return complete_event_;
    }

    TaskState Task::state() const {
        return state_;
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
