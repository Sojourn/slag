#pragma once

#include "slag/core/service_interface.h"
#include "slag/system/operation.h"

namespace slag {

    template<OperationType type>
    class OperationHandle {
    private:
        template<ServiceType service_type>
        friend class ServiceInterface;

        explicit OperationHandle(Operation<type>& operation);

    public:
        OperationHandle();
        OperationHandle(OperationHandle&& other);
        OperationHandle(const OperationHandle&) = delete;
        ~OperationHandle();

        OperationHandle& operator=(OperationHandle&& that);
        OperationHandle& operator=(const OperationHandle&) = delete;

        explicit operator bool() const;

        Operation<type>& operator*();
        const Operation<type>& operator*() const;
        Operation<type>* operator->();
        const Operation<type>* operator->() const;

        void reset();

    private:
        Operation<type>* operation_;
    };

}

#include "operation_handle.hpp"
