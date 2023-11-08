#pragma once

#include <sys/mman.h>
#include "slag/util.h"
#include "slag/layer.h"
#include "slag/logging.h"
#include "slag/core/service.h"
#include "slag/core/service_interface.h"
#include "slag/memory/memory_service_interface.h"

namespace slag {

    // Manages a set of memory banks which are given out by
    // root allocators.

    class ServiceRegistry;

    class MemoryService : public ServiceInterface<ServiceType::MEMORY> {
    public:
        explicit MemoryService(ServiceRegistry& service_registry);

        void start_service() override final;
        void stop_service() override final;
    };

}
