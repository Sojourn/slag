#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/core/service.h"

namespace slag {

    class ServiceRegistry;

    template<ServiceType type>
    class ServiceInterface;

    using SystemServiceInterface = ServiceInterface<ServiceType::SYSTEM>;
    using SchedulerServiceInterface = ServiceInterface<ServiceType::SCHEDULER>;

    SystemServiceInterface& get_system_service();
    SchedulerServiceInterface& get_scheduler_service();

    SystemServiceInterface& get_system_service(ServiceRegistry& service_registry);
    SchedulerServiceInterface& get_scheduler_service(ServiceRegistry& service_registry);

}
