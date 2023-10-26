#include "slag/system/system_service.h"

namespace slag {

    SystemService::SystemService(ServiceRegistry& service_registry)
        : SystemServiceInterface{service_registry}
        , reactor_{pending_submissions()}
    {
    }

    void SystemService::start_service() {
        ServiceInterface<ServiceType::SYSTEM>::start_service();
    }

    void SystemService::stop_service() {
        while (metrics().operations.total_active_count() > 0) {
            bool non_blocking = false;
            poll(non_blocking);
        }

        ServiceInterface<ServiceType::SYSTEM>::stop_service();
    }

    bool SystemService::poll(bool non_blocking) {
        return reactor_.poll(non_blocking);
    }

}
