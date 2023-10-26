#pragma once

#include <array>
#include <memory>
#include "slag/core/service.h"
#include "slag/core/service_interface.h"

namespace slag {

    class ServiceRegistry {
    public:
        ServiceRegistry();

        void register_service(Service& service);

        template<ServiceType type>
        ServiceInterface<type>& get_service();
        Service& get_service(ServiceType type);

        template<ServiceType type>
        const ServiceInterface<type>& get_service() const;
        const Service& get_service(ServiceType type) const;

        template<typename Visitor>
        void for_each_service(Visitor&& visitor);

        template<typename Visitor>
        void for_each_service_reverse(Visitor&& visitor);

    private:
        std::array<Service*, SERVICE_TYPE_COUNT> services_;
    };

}

#include "service_registry.hpp"
