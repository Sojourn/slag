#pragma once

#include <array>
#include <memory>
#include "slag/core/service.h"
#include "slag/core/service_interface.h"

namespace slag {

    class ServiceRegistry {
    public:
        template<typename ServiceImpl, typename... Args>
        void initialize(Args&&... args);

        template<ServiceType type>
        ServiceInterface<type>& get_service();
        Service& get_service(ServiceType type);

        template<ServiceType type>
        const ServiceInterface<type>& get_service() const;
        const Service& get_service(ServiceType type) const;

        void start_services();
        void stop_services();

    private:
        std::array<std::unique_ptr<Service>, SERVICE_TYPE_COUNT> services_;
    };

}

#include "service_registry.hpp"
