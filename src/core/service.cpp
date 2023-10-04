#include "slag/core/service.h"

namespace slag {

    Service::Service(ServiceType type)
        : type_{type}
    {
    }

    ServiceType Service::type() const {
        return type_;
    }

}
