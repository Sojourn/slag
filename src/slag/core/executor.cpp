#include "executor.h"
#include <stdexcept>

namespace slag {

    Event& Executor::runnable_event() {
        return selector_.readable_event();
    }

    void Executor::schedule(Task& task) {
        selector_.insert<PollableType::RUNNABLE>(task);
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
                // TODO: Better error handling / logging.
                std::cerr << ex.what() << std::endl;
                task->cancel();
            }

            if (!task->is_complete()) {
                schedule(*task);
            }
        }
    }

}
