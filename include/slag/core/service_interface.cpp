#include "slag/core/service_interface.h"
#include "slag/core/service_registry.h"
#include "slag/event_loop.h"
#include <cassert>

namespace slag {

    static ServiceRegistry& get_service_registry() {
        return get_event_loop().service_registry();
    }

    SystemServiceInterface& get_system_service() {
        return get_system_service(get_service_registry());
    }

    SchedulerServiceInterface& get_scheduler_service() {
        return get_scheduler_service(get_service_registry());
    }

    SystemServiceInterface& get_system_service() {
        return service_registry.get_service<ServiceType::SYSTEM>();
    }

    SchedulerServiceInterface& get_scheduler_service() {
        return service_registry.get_service<ServiceType::SCHEDULER>();
    }

}
