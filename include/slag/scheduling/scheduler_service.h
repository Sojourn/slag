#pragma once

#include "slag/core/service.h"
#include "slag/core/service_interface.h"

namespace slag {

    class SchedulerService : public ServiceInterface<ServiceType::SCHEDULER> {
    public:
        void start_service() override final {
        }

        void stop_service() override final {
        }

    private:
        Scheduler               root_;
        std::vector<Scheduler*> stack_;
    };

}
