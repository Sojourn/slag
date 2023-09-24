#include "slag/core/executor.h"
#include "slag/core/domain.h"
#include "slag/logging.h"
#include <stdexcept>

namespace slag {

    class ExecutorActivation {
        ExecutorActivation(ExecutorActivation&&) = delete;
        ExecutorActivation(const ExecutorActivation&) = delete;
        ExecutorActivation& operator=(ExecutorActivation&&) = delete;
        ExecutorActivation& operator=(const ExecutorActivation&) = delete;

    public:
        ExecutorActivation(Region& region, Executor& executor)
            : region_{region}
            , executor_{executor}
        {
            region_.enter_executor(executor_);
        }

        ~ExecutorActivation() {
            region_.leave_executor(executor_);
        }

    private:
        Region&   region_;
        Executor& executor_;
    };

    Executor::Executor(const Quantum& quantum)
        : region_{region()}
        , quantum_{quantum}
    {
    }

    auto Executor::quantum() const -> const Quantum& {
        return quantum_;
    }

    void Executor::set_quantum(const Quantum& quantum) {
        quantum_ = quantum;
    }

    void Executor::insert(Task& task) {
        Event& event = task.runnable_event();
        event.set_user_data(&task);
        selector_.insert(event);
    }

    void Executor::remove(Task& task) {
        Event& event = task.runnable_event();
        selector_.remove(event);
    }

    Event& Executor::runnable_event() {
        return selector_.readable_event();
    }

    void Executor::run() {
        ExecutorActivation activation{region_, *this};
        {
            auto deadline = std::chrono::steady_clock::now() + quantum_;

            while (Event* event = selector_.select()) {
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
                    insert(*task);
                }

                // Check if the deadline was reached and we need to yield.
                if (deadline <= std::chrono::steady_clock::now()) {
                    break;
                }
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

                error(
                    "[Executor] task:{} threw:'{}' while running."
                    , static_cast<const void*>(&task)
                    , ex.what()
                );
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
