#pragma once

#include "slag/core/service.h"
#include "slag/core/service_interface.h"

namespace slag {

    class ServiceRegistry;

    template<>
    class ServiceInterface<ServiceType::MEMORY> : public Service {
    public:
        explicit ServiceInterface(ServiceRegistry& service_registry)
            : Service(ServiceType::MEMORY, service_registry)
        {
        }

        void start_service() override {
        }

        void stop_service() override {
        }
    };

}
