#include "executor.h"
#include <stdexcept>

namespace slag {

    Event& Executor::runnable_event() {
        return selector_.readable_event();
    }

    void Executor::schedule(Task& task) {
        Event& event = task.runnable_event();
        if (event.is_linked()) {
            event.unlink();
        }

        selector_.insert(event, &task);
    }

    void Executor::run(const size_t budget) {
        for (size_t i = 0; i < budget; ++i) {
            Event* event = selector_.select();
            if (!event) {
                break;
            }

            Task* task = static_cast<Task*>(event->user_data());
            if (task->is_complete()) {
                continue; // Canceled, probably.
            }

            try {
                task->run();
            }
            catch (const std::exception& ex) {
                assert(false);
                task->cancel();
            }

            if (task->is_complete()) {
                // The task will be cleaned up by another task
                // waiting on this task's completion event.
            }
            else {
                schedule(*task);
            }
        }
    }

}
