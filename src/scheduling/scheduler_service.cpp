#include "slag/scheduling/scheduler_service.h"
#include <cassert>

namespace slag {

    SchedulerService::SchedulerService(ServiceRegistry& service_registry)
        : ServiceInterface<ServiceType::SCHEDULER>(service_registry)
    {
    }

    void SchedulerService::start_service() {
        ServiceInterface<ServiceType::SCHEDULER>::start_service();
    }

    void SchedulerService::stop_service() {
        ServiceInterface<ServiceType::SCHEDULER>::stop_service();
    }

    bool SchedulerService::is_quiescent() const {
        if (high_priority_executor_.is_runnable()) {
            return false;
        }
        if (idle_priority_executor_.is_runnable()) {
            return false;
        }

        // None of our executors have runnable tasks. We are quiescent.
        return true;
    }

    void SchedulerService::schedule_task(Task& task) {
        Executor* executor = nullptr;
        switch (task.priority()) {
            case TaskPriority::SAME: {
                executor = &active_executor();
                break;
            }
            case TaskPriority::HIGH: {
                executor = &high_priority_executor_;
                break;
            }
            case TaskPriority::IDLE: {
                executor = &idle_priority_executor_;
                break;
            }
        }

        executor->insert(task);
    }

    void SchedulerService::run() {
        if (high_priority_executor_.is_runnable()) {
            high_priority_executor_.run();
        }
        else if (idle_priority_executor_.is_runnable()) {
            idle_priority_executor_.run();
        }
        else {
            assert(is_quiescent());
        }
    }

}
