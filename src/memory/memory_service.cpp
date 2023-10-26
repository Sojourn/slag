#include "slag/memory/memory_service.h"

namespace slag {

    MemoryService::MemoryService(ServiceRegistry& service_registry)
        : ServiceInterface<ServiceType::MEMORY>(service_registry)
    {
    }

    void MemoryService::start_service() {
    }

    void MemoryService::stop_service() {
    }

}
