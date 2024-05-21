#pragma once

#include <bitset>
#include "slag/core.h"
#include "operation_types.h"
#include "operation_user_data.h"

namespace slag {

    template<>
    class Resource<ResourceType::OPERATION>
        : public ResourceBase
        , public Pollable<PollableType::READABLE>
        , public Pollable<PollableType::WRITABLE>
    {
    public:
        explicit Resource(const OperationType operation_type)
            : ResourceBase(ResourceType::OPERATION)
            , operation_type_(operation_type)
            , abandoned_(false)
            , daemonized_(false)
        {
        }

        [[nodiscard]]
        OperationType operation_type() const {
            return operation_type_;
        }

        [[nodiscard]]
        bool is_quiescent() const {
            return working_slots_.none();
        }

        [[nodiscard]]
        bool is_abandoned() const {
            return abandoned_;
        }

        void abandon() {
            abandoned_ = true;
        }

        [[nodiscard]]
        bool is_daemonized() const {
            return daemonized_;
        }

        void daemonize() {
            daemonized_ = true;
        }

        // TODO: Extract this logic so it can be tested.
        // TODO: Rename these to use acquire/release verbage.
        OperationSlot allocate_slot() {
            if (working_slots_.all()) {
                // This should never happen. We should be able to statically
                // determine the maximum number of operations that can be outstanding.
                abort();
            }

            uint32_t mask = working_slots_.to_ulong();
            OperationSlot slot = static_cast<OperationSlot>(__builtin_ctzll(mask));
            working_slots_.set(slot);

            return slot;
        }

        void deallocate_slot(OperationSlot slot) {
            working_slots_.reset(slot);
        }

        virtual void cancel() = 0;
        virtual void prepare(struct io_uring_sqe& sqe, OperationUserData& user_data) = 0;
        virtual void handle_result(int32_t result, bool complete, OperationUserData user_data) = 0;

    private:
        OperationType   operation_type_;
        bool            abandoned_;
        bool            daemonized_;
        std::bitset<16> working_slots_;
    };

    using Operation       = Resource<ResourceType::OPERATION>;
    using OperationHandle = mantle::Handle<Operation>;

}
