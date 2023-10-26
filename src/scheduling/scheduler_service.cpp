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
        return !(busy_executor_.is_runnable() || idle_executor_.is_runnable());
    }

    void SchedulerService::schedule_task(Task& task) {
        Executor* executor = nullptr;
        switch (task.priority()) {
            case TaskPriority::SAME: {
                executor = &active_executor();
                break;
            }
            case TaskPriority::BUSY: {
                executor = &busy_executor_;
                break;
            }
            case TaskPriority::IDLE: {
                executor = &idle_executor_;
                break;
            }
        }

        executor->insert(task);
    }

    void SchedulerService::run() {
        if (busy_executor_.is_runnable()) {
            busy_executor_.run();
        }
        else if (idle_executor_.is_runnable()) {
            idle_executor_.run();
        }
        else {
            assert(is_quiescent());
        }
    }

}
