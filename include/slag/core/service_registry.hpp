#include "slag/system/system_service_interface.h"
#include "slag/memory/memory_service_interface.h"
#include "slag/scheduling/scheduler_service_interface.h"
#include <tuple>
#include <utility>
#include <stdexcept>
#include <cassert>

namespace slag {

    inline ServiceRegistry::ServiceRegistry() {
        for (auto&& service: services_) {
            service = nullptr;
        }
    }

    inline void ServiceRegistry::register_service(Service& service) {
        Service*& service_pointer = services_[to_index(service.type())];
        assert(!service_pointer);
        service_pointer = &service;
    }

    template<ServiceType type>
    inline ServiceInterface<type>& ServiceRegistry::get_service() {
        return static_cast<ServiceInterface<type>&>(*services_[to_index(type)]);
    }

    inline Service& ServiceRegistry::get_service(ServiceType type) {
        return *services_[to_index(type)];
    }

    template<ServiceType type>
    inline const ServiceInterface<type>& ServiceRegistry::get_service() const {
        return static_cast<const ServiceInterface<type>&>(*services_[to_index(type)]);
    }

    inline const Service& ServiceRegistry::get_service(ServiceType type) const {
        return *services_[to_index(type)];
    }

    template<typename Visitor>
    inline void ServiceRegistry::for_each_service(Visitor&& visitor) {
        auto visit = [&]<size_t... I>(std::index_sequence<I...>) {
            (visitor(get_service<static_cast<ServiceType>(I)>()), ...);
        };

        visit(std::make_index_sequence<SERVICE_TYPE_COUNT>());
    }

    template<typename Visitor>
    inline void ServiceRegistry::for_each_service_reverse(Visitor&& visitor) {
        auto visit = [&]<size_t... I>(std::index_sequence<I...>) {
            (
                visitor(
                    get_service<static_cast<ServiceType>(SERVICE_TYPE_COUNT - I - 1)>()
                ), ...
            );
        };

        visit(std::make_index_sequence<SERVICE_TYPE_COUNT>());
    }

}
