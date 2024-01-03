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
        explicit Resource(ResourceDescriptor descriptor, OperationType type)
            : ResourceBase(descriptor)
            , type_(type)
            , daemonized_(false)
        {
        }

        OperationType type() const {
            return type_;
        }

        bool is_quiescent() const {
            return working_slots_.none();
        }

        bool is_daemonized() const {
            return daemonized_;
        }

        void daemonize() {
            daemonized_ = true;
        }

        // TODO: Extract this logic so it can be tested.
        // TODO: Rename these to use acquire/release verbage.
        OperationSlot allocate_slot() {
            assert(!working_slots_.all());
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
        OperationType   type_;
        bool            daemonized_;
        std::bitset<32> working_slots_;
    };

    using Operation = Resource<ResourceType::OPERATION>;

}
