#include "slag/postal/services/worker_service.h"

namespace slag {

    template<typename Stack>
    WorkerService<Stack>::WorkerService()
        : Service{ServiceType::WORKER}
    {
        selector_.insert(executor_.runnable_event(), nullptr);
    }

    template<typename Stack>
    template<typename T, typename... Args>
    void WorkerService<Stack>::spawn(Args&&... args) {
        auto&& task = tasks_.emplace_back(
            std::make_unique<T>(std::forward<Args>(args)...)
        );

        executor_.insert(*task);
        selector_.insert(task->complete_event(), task.get());
    }

    template<typename Stack>
    void WorkerService<Stack>::start() {
        info("[WorkerService] starting");

        set_service_state(ServiceState::STARTING);
    }

    template<typename Stack>
    void WorkerService<Stack>::stop() {
        info("[WorkerService] stopping");

        set_service_state(ServiceState::STOPPING);
    }

    template<typename Stack>
    Event& WorkerService<Stack>::runnable_event() {
        return selector_.readable_event();
    }

    template<typename Stack>
    void WorkerService<Stack>::run() {
        if (is_service_starting()) {
            set_service_state(ServiceState::RUNNING);
            return;
        }
        if (is_service_stopping()) {
            // TODO: think more about task cancelation, and a more
            //       graceful way to do this.
            tasks_.clear();

            set_service_state(ServiceState::STOPPED);
            return;
        }

        if (Event* event = selector_.select()) {
            if (void* user_data = event->user_data()) {
                reap_task(*reinterpret_cast<Task*>(user_data));
            }
            else {
                while (executor_.is_runnable()) {
                    executor_.run();
                }

                selector_.insert(executor_.runnable_event(), nullptr);
            }
        }
    }

    template<typename Stack>
    void WorkerService<Stack>::reap_task(Task& task) {
        for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
            if (it->get() == &task) {
                tasks_.erase(it);
                break;
            }
        }
    }

}
