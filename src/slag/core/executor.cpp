#include "executor.h"
#include <stdexcept>

namespace slag {

    Event& Executor::runnable_event() {
        return scheduler_.readable_event();
    }

    void Executor::schedule(Task& task) {
        scheduler_.insert<PollableType::RUNNABLE>(task);
    }

    void Executor::run() {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::microseconds(10);

        while (Event* event = scheduler_.select()) {
            // Fetch our stashed task pointer.
            Task* task = static_cast<Task*>(event->user_data());
            assert(task);

            // Check if the task completed externally.
            if (task->is_complete()) {
                assert(task->is_success() || task->is_failure());
                continue;
            }

            // Run the task and reschedule if it doesn't complete.
            run_until(*task, deadline);
            if (!task->is_complete()) {
                schedule(*task);
            }

            // Check if the deadline was reached and we need to yield.
            if (deadline <= std::chrono::steady_clock::now()) {
                break;
            }
        }
    }

    void Executor::run_until(Task& task, const Deadline& deadline) {
        task.set_state(TaskState::RUNNING);

        // A task is allowed to execute for the entire quantum if it
        // still has work to do.
        while (true) {
            try {
                task.run();
            }
            catch (const std::exception& ex) {
                assert(!task.is_complete());

                // Force the task into a failed state.
                task.set_state(TaskState::FAILURE);

                // TODO: Log instead.
                abort();
            }

            if (is_terminal(task.state())) {
                break; // Task is complete.
            }
            else {
                bool should_yield = false;
                should_yield = should_yield || !task.is_runnable();
                should_yield = should_yield || (deadline <= std::chrono::steady_clock::now());
                if (should_yield) {
                    task.set_state(TaskState::WAITING);
                    break;
                }
            }
        }

        assert(!task.is_running());
    }

}
