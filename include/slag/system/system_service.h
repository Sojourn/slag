#pragma once

#include "slag/core/service.h"
#include "slag/core/service_interface.h"
#include "slag/system/reactor.h"
#include "slag/system/operation.h"
#include "slag/system/system_service_interface.h"

namespace slag {

    class ServiceRegistry;

    class SystemService
        : public ServiceInterface<ServiceType::SYSTEM>
        , public InterruptHandler
    {
    public:
        explicit SystemService(ServiceRegistry& service_registry);

        void start_service() override final;
        void stop_service() override final;

        bool poll(bool non_blocking) override final;

    private:
        void handle_operation_started(OperationBase& operation_base) override final;
        void handle_operation_abandoned(OperationBase& operation_base) override final;
        void handle_operation_daemonized(OperationBase& operation_base) override final;

    private:
        Reactor reactor_;
    };

}
