#include "slag/postal/executor.h"
#include "slag/postal/domain.h"
#include "slag/logging.h"
#include <stdexcept>

namespace slag::postal {

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
        // Check if any of the events tasks are waiting on have become ready.
        Event* event = selector_.select();
        if (!event) {
            return;
        }

        // Fetch our stashed task pointer.
        Task* task = static_cast<Task*>(event->user_data());
        assert(task);
        assert(task->is_waiting());

        // Run the task, and if it completes, we're done.
        run_until(*task, std::chrono::steady_clock::now() + quantum_);
        if (is_terminal(task->state())) {
            return;
        }

        // Fetch the event again in case it has changed. It could be
        // waiting on something new.
        event = &task->runnable_event();
        event->set_user_data(task);

        selector_.insert(*event);
    }

    void Executor::run_until(Task& task, const Deadline& deadline) {
        task.set_state(TaskState::RUNNING);
        region_.enter_executor(*this);

        while (true) {
            try {
                task.run();
            }
            catch (const std::exception& ex) {
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
                should_yield |= !task.is_runnable();
                should_yield |= deadline <= std::chrono::steady_clock::now();
                if (should_yield) {
                    task.set_state(TaskState::WAITING);
                    break;
                }
            }
        }

        region_.leave_executor(*this);
        assert(!task.is_running());
    }

}
