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

        // Run the task until the quantum has elapsed or it starts waiting for something.
        // This will return the event that the task is currently waiting for.
        event = &run_until(*task, std::chrono::steady_clock::now() + quantum_);

        // Schedule the task to run once the event it is waiting on is ready.
        event->set_user_data(task);
        selector_.insert(*event);
    }

    Event& Executor::run_until(Task& task, const Deadline& deadline) {
        Event* event = nullptr;
        region_.enter_executor(*this);

        while (true) {
            try {
                task.run();
            }
            catch (const std::exception& ex) {
                fatal(
                    "[Executor] task:{} threw:'{}' while running. Evicting it."
                    , static_cast<const void*>(&task)
                    , ex.what()
                );

                // Options:
                //   1: Penalize the task.
                //        - Cancel the rest of the quantum by returning early. The task is
                //          probably borked (since it threw) and may tell us to wait on the
                //          wrong event and get stuck.
                //   2: Evict it.
                //        - Don't run it again. This is likely to cause something to get stuck.
                //   3: Boom.
                //        - Catch bugs early by aborting the program to generate a core dump.
                //   4: Add a status to the task.
                //
                abort(); // Boom.
            }

            // Refresh the event that the task is waiting for. It may have changed.
            event = &task.runnable_event();
            if (!event->is_set()) {
                break; // The task is waiting.
            }

            // Check if we need to interrupt the task.
            if (deadline <= std::chrono::steady_clock::now()) {
                break; // The quantum has elapsed, stop scheduling it.
            }
        }

        region_.leave_executor(*this);
        return *event;
    }

}
