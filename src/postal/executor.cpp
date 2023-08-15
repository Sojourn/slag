#include "slag/postal/executor.h"
#include <chrono>

namespace slag::postal {

    Executor::Executor(const std::chrono::duration& quantum)
        : quantum_{quantum}
    {
    }

    const std::chrono::duration& Executor::quantum() const {
        return quantum_;
    }

    void Executor::set_quantum(const std::chrono::duration& quantum) {
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
        // The way we are using the selector will cause us to prefetch the next
        // ready event. It would be nice if we could peek with different offsets to
        // prefetch the event, and the task in staggered run calls.
        Event* event = selector_.select();
        if (!event) {
            return; // Nothing is currently runnable.
        }

        // Grab a pointer to the task that we stashed in the user data of
        // the event it was waiting for.
        Task* task = static_cast<Task*>(event->user_data());
        assert(task);

        // Compute when the task should be interrupted. This cannot be changed
        // after the task has started running (set_quantum will not impact this task).
        auto deadline = std::chrono::steady_clock::now() + quantum_;

        while (true) {
            task->run();

            // Refresh the event that the task is waiting for. It may have changed.
            event = &task->runnable_event();
            if (!event->is_set()) {
                break; // The task is waiting.
            }

            // Check if we need to interrupt the task.
            if (deadline <= std::chrono::steady_clock::now()) {
                break; // The quantum has elapsed, stop scheduling it.
            }
        }

        // Schedule the task to run once the event it is waiting on is ready.
        event->set_user_data(task);
        selector_.insert(*event);
    }

}
