#pragma once

#include "slag/core/selector.h"
#include "slag/core/service.h"
#include "slag/core/service_interface.h"
#include "slag/scheduling/scheduler_service_interface.h"

namespace slag {

    class ServiceRegistry;

    class SchedulerService : public ServiceInterface<ServiceType::SCHEDULER> {
    public:
        SchedulerService(ServiceRegistry& service_registry);

        void start_service() override final;
        void stop_service() override final;

        bool is_quiescent() const override final;
        void schedule_task(Task& task) override final;
        void run() override final;

    private:
        Executor high_priority_executor_;
        Executor idle_priority_executor_;
    };

}
