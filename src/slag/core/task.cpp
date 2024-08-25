#include "task.h"
#include "executor.h"
#include "slag/context.h"
#include "slag/event_loop.h"
#include <stdexcept>

namespace slag {

    Task::Task(TaskPriority priority)
        : Task(get_event_loop().executor(priority))
    {
    }

    Task::Task(Executor& executor)
        : state_{TaskState::WAITING}
    {
        runnable_event_.set();

        // This will use `Task::runnable_event` and not an overridden one since
        // our dynamic type is still `Task`.
        executor.schedule(*this);
    }

    Event& Task::runnable_event() {
        return runnable_event_;
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

    void Task::cancel() {
        kill();
    }

    void Task::kill() {
        if (is_valid_transition(state_, TaskState::FAILURE)) {
            set_state(TaskState::FAILURE);
        }
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
