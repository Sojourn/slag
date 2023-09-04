#pragma once

#include <liburing.h>        
#include "slag/intrusive_queue.h"
#include "slag/postal/event.h"
#include "slag/postal/pollable.h"
#include "slag/postal/operation_types.h"

namespace slag::postal {

    class Reactor;

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

        Reactor& reactor();
        const Reactor& reactor() const;

        Event& writable_event() override final;
        Event& readable_event() override final;

        // Attempt to cancel the operation.
        virtual void cancel() = 0;

    public:
        using Slot     = int8_t;
        using SlotMask = uint16_t;

        static constexpr Slot SLOT_COUNT = 16;

        void* make_user_data(Slot slot);
        std::pair<void*, Slot> produce_slot();
        static std::pair<OperationBase*, Slot> consume_slot(void* user_data);

    protected:
        OperationBase(OperationType type, Reactor& reactor);
        ~OperationBase();

        // Returns true if this operation has nothing in-flight.
        bool is_quiescent() const;

        // Returns true if there are no handles associated with this operation.
        bool is_abandoned() const;

    private: // API for the reactor.
        friend class Reactor;

        virtual void prepare(Slot slot, struct io_uring_sqe& sqe) = 0;
        virtual void handle_result(Slot slot, int32_t result) = 0;

    private: // API for handles.
        template<OperationType type>
        friend class OperationHandle;

        void abandon();

    private:
        OperationType      type_;
        bool               abandoned_;
        SlotMask           slot_mask_;
        Event              writable_event_;
        Event              readable_event_;
        Reactor&           reactor_;
        IntrusiveQueueNode node_;
    };
    static_assert(OperationBase::SLOT_COUNT <= alignof(OperationBase));
    static_assert(OperationBase::SLOT_COUNT <= (sizeof(OperationBase::SlotMask) * 8));

    // An individual, asynchronous system call.
    class PrimitiveOperation
        : public OperationBase
        , public Pollable<PollableType::COMPLETE>
    {
    protected:
        using OperationBase::OperationBase;
    };

    // An operation composed of other operations.
    // This could be a sequence (socket + connect), or wrapping an
    // existing operation (receive_exactly).
    // class CompositeOperation
    //     : public OperationBase
    //     , public Task
    // {
    // protected:
    //     using OperationBase::OperationBase;
    // };

    template<OperationType type>
    class Operation;

}
