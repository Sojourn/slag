#include "slag/postal/operation_base.h"
#include "slag/postal/reactor.h"

namespace slag::postal {

    OperationBase::OperationBase(OperationType type, Reactor& reactor)
        : type_{type}
        , abandoned_{false}
        , daemonized_{false}
        , reactor_{reactor}
    {
    }

    OperationBase::~OperationBase() {
        assert(is_quiescent());
    }

    OperationType OperationBase::type() const {
        return type_;
    }

    Reactor& OperationBase::reactor() {
        return reactor_;
    }

    const Reactor& OperationBase::reactor() const {
        return reactor_;
    }

    Event& OperationBase::writable_event() {
        return writable_event_;
    }

    bool OperationBase::is_quiescent() const {
        return slot_mask_ == 0;
    }

    bool OperationBase::is_abandoned() const {
        return abandoned_;
    }

    void OperationBase::abandon() {
        if (daemonized_) {
            // Allow daemonized operations to continue running.
        }
        else {
            cancel();
        }

        abandoned_ = true;
        reactor_.handle_abandoned(*this);
    }

    void OperationBase::daemonize() {
        daemonized_ = true;
    }

    void* OperationBase::make_user_data(Slot slot) {
        return reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(this) + slot
        );
    }

    auto OperationBase::produce_slot() -> std::pair<void*, Slot> {
        if (slot_mask_ == std::numeric_limits<SlotMask>::max()) {
            return std::make_pair(nullptr, Slot{-1});
        }

        Slot slot = static_cast<Slot>(__builtin_ctzll(~slot_mask_));
        slot_mask_ |= (1u << slot);

        return std::make_pair(make_user_data(slot), slot);
    }

    auto OperationBase::consume_slot(void* user_data) -> std::pair<OperationBase*, Slot> {
        std::pair<OperationBase*, Slot> result = peek_slot(user_data);

        auto&& [operation_base, slot] = result;
        operation_base->slot_mask_ &= ~(1u << slot);

        return result;
    }

    auto OperationBase::peek_slot(void* user_data) -> std::pair<OperationBase*, Slot> {
        auto packed_value = reinterpret_cast<uintptr_t>(user_data);

        OperationBase* operation_base = reinterpret_cast<OperationBase*>(packed_value & ~(SLOT_COUNT - 1));
        Slot slot = static_cast<Slot>(packed_value & (SLOT_COUNT - 1));

        return std::make_pair(operation_base, slot);
    }

}
