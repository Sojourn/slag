#pragma once

#include "slag/core/task.h"

#define SLAG_SERVICE_TYPES(X) \
    X(SYSTEM)                 \
    X(MEMORY)                 \
    X(SCHEDULER)              \

namespace slag {

    class ServiceRegistry;

    enum class ServiceType {
    #define X(SLAG_SERVICE_TYPE) SLAG_SERVICE_TYPE,
        SLAG_SERVICE_TYPES(X)
    #undef X
    };

    constexpr size_t SERVICE_TYPE_COUNT = 0
    #define X(SLAG_SERVICE_TYPE) + 1
        SLAG_SERVICE_TYPES(X)
    #undef X
    ;

    constexpr inline size_t to_index(ServiceType type) {
        return static_cast<size_t>(type);
    }

    class Service {
        Service(Service&&) = delete;
        Service(const Service&) = delete;
        Service& operator=(Service&&) = delete;
        Service& operator=(const Service&) = delete;

    public:
        Service(ServiceType type, ServiceRegistry& service_registry);
        virtual ~Service() = default;

        ServiceType type() const;

        ServiceRegistry& service_registry();
        const ServiceRegistry& service_registry() const;

        virtual void start_service() = 0;
        virtual void stop_service() = 0;

    private:
        ServiceType      type_;
        ServiceRegistry& service_registry_;
    };

}
