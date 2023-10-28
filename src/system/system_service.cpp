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

    void SystemService::handle_operation_started(OperationBase& operation_base) {
        ServiceInterface<ServiceType::SYSTEM>::handle_operation_started(operation_base);

        // Immediately forward operations to the reactor for submission.
        pending_submissions().insert<PollableType::WRITABLE>(operation_base);
    }

    void SystemService::handle_operation_abandoned(OperationBase& operation_base) {
        ServiceInterface<ServiceType::SYSTEM>::handle_operation_abandoned(operation_base);
    }

    void SystemService::handle_operation_daemonized(OperationBase& operation_base) {
        ServiceInterface<ServiceType::SYSTEM>::handle_operation_daemonized(operation_base);
    }

}
