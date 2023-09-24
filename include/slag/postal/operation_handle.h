#pragma once

#include "slag/postal/operation.h"

namespace slag {

    template<OperationType type>
    class OperationHandle {
    private:
        friend class Reactor;

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
