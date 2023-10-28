#include "slag/core/service.h"
#include "slag/core/service_registry.h"

namespace slag {

    Service::Service(ServiceType type, ServiceRegistry& service_registry)
        : type_{type}
        , service_registry_{service_registry}
    {
        service_registry_.register_service(*this);
    }

    ServiceType Service::type() const {
        return type_;
    }

    ServiceRegistry& Service::service_registry() {
        return service_registry_;
    }

    const ServiceRegistry& Service::service_registry() const {
        return service_registry_;
    }

}
