#include "slag/system/operation_base.h"
#include "slag/system/reactor.h"
#include "slag/system/system_service_interface.h"

namespace slag {

    OperationBase::OperationBase(OperationType type, SystemServiceInterface& system_service)
        : type_{type}
        , abandoned_{false}
        , daemonized_{false}
        , system_service_{system_service}
    {
    }

    OperationBase::~OperationBase() {
        assert(is_quiescent());
    }

    OperationType OperationBase::type() const {
        return type_;
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

    bool OperationBase::is_daemonized() const {
        return daemonized_;
    }

    void OperationBase::abandon() {
        if (abandoned_) {
            return; // Guard against being abandoned multiple times.
        }

        if (daemonized_) {
            // Allow daemonized operations to continue running.
        }
        else {
            cancel();
        }

        abandoned_ = true;
        system_service_.handle_operation_abandoned(*this);
    }

    void OperationBase::daemonize() {
        if (daemonized_) {
            return; // Guard against being daemonized multiple times.
        }

        daemonized_ = true;
        system_service_.handle_operation_daemonized(*this);
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
