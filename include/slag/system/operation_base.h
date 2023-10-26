#pragma once

#include <liburing.h>        
#include "slag/core/event.h"
#include "slag/core/pollable.h"
#include "slag/core/service_interface.h"
#include "slag/collection/intrusive_queue.h"
#include "slag/system/operation_types.h"

namespace slag {

    class Reactor;

    template<OperationType type>
    class Operation;

    // The higher than usual alignment is used to allocate a few
    // padding bits (tagged slots).
    class alignas(16) OperationBase
        : public Pollable<PollableType::READABLE>
        , public Pollable<PollableType::WRITABLE>
    {
        OperationBase(OperationBase&&) = delete;
        OperationBase(const OperationBase&) = delete;
        OperationBase& operator=(OperationBase&&) = delete;
        OperationBase& operator=(const OperationBase&) = delete;

    public:
        OperationType type() const;

        // A 'writable' operation wants to submit something.
        Event& writable_event() override final;

        // Attempt to cancel the operation.
        virtual void cancel() = 0;

        // Prevent cancelation when the handle for this is destroyed.
        void daemonize();

    public:
        using Slot     = int8_t;
        using SlotMask = uint16_t;

        static constexpr Slot SLOT_COUNT = 16;

        void* make_user_data(Slot slot);
        std::pair<void*, Slot> produce_slot();
        static std::pair<OperationBase*, Slot> consume_slot(void* user_data);
        static std::pair<OperationBase*, Slot> peek_slot(void* user_data);

    protected:
        template<ServiceType service_type>
        friend class ServiceInterface;

        OperationBase(OperationType type, SystemServiceInterface& system_service);
        ~OperationBase();

        // Returns true if this operation has nothing in-flight.
        bool is_quiescent() const;

        // Returns true if there are no handles associated with this operation.
        bool is_abandoned() const;

        // Returns true if this operation has been daemonized. It will run
        // to completion even if it is abandoned.
        bool is_daemonized() const;

    private: // API for the reactor.
        friend class Reactor;

        virtual void prepare(Slot slot, struct io_uring_sqe& sqe) = 0;
        virtual void handle_result(Slot slot, int32_t result, bool more) = 0;

    private: // API for handles.
        template<OperationType type>
        friend class OperationHandle;

        void abandon();

    private:
        OperationType           type_;
        bool                    abandoned_;
        bool                    daemonized_;
        SlotMask                slot_mask_;
        Event                   writable_event_;
        SystemServiceInterface& system_service_;
        IntrusiveQueueNode      node_;
    };
    static_assert(OperationBase::SLOT_COUNT <= alignof(OperationBase));
    static_assert(OperationBase::SLOT_COUNT <= (sizeof(OperationBase::SlotMask) * 8));

}
