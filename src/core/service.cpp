#include "slag/core/service.h"

namespace slag {

    Service::Service(ServiceType type, ServiceRegistry& service_registry)
        : type_{type}
        , service_registry_{service_registry}
    {
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
