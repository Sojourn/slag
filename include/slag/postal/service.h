#pragma once

#include "slag/postal/task.h"

#define SLAG_SERVICE_TYPES(X) \
    X(SYSTEM)                 \
    X(MEMORY)                 \
    X(LOGGER)                 \
    X(CHRONO)                 \
    X(POSTAL)                 \
    X(WORKER)                 \

namespace slag::postal {

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

    enum class ServiceState {
        INITIAL,
        STARTING,
        RUNNING,
        STOPPING,
        STOPPED,
    };

    class Service : public Task {
    public:
        explicit Service(ServiceType type)
            : type_{type}
            , state_{ServiceState::INITIAL}
        {
        }

        ServiceType service_type() const {
            return type_;
        }

        ServiceState service_state() const {
            return state_;
        }

        bool is_service_starting() const {
            return state_ == ServiceState::STARTING;
        }

        bool is_service_running() const {
            return state_ == ServiceState::RUNNING;
        }

        bool is_service_stopping() const {
            return state_ == ServiceState::STOPPING;
        }

        bool is_service_stopped() const {
            return state_ == ServiceState::STOPPED;
        }

    protected:
        void set_service_state(ServiceState state) {
            assert(static_cast<size_t>(state_) + 1 == static_cast<size_t>(state));

            state_ = state;
        }

    private:
        ServiceType  type_;
        ServiceState state_;
    };

}
