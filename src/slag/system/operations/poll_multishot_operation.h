#pragma once

#include <sys/poll.h>
#include <liburing.h>
#include <variant>
#include "slag/core.h"
#include "slag/system/operation.h"
#include "slag/system/file_descriptor.h"

namespace slag {

    class PollMultishotOperation final : public Operation {
    public:
        PollMultishotOperation(const Ref<FileDescriptor>& file_descriptor)
            : Operation(OperationType::POLL_MULTISHOT)
            , file_descriptor_(file_descriptor)
            , result_(-EAGAIN)
        {
        }

        int32_t result() const {
            return result_;
        }

    private:
        void prepare_operation(struct io_uring_sqe& io_sqe) override {
            io_uring_prep_poll_multishot(&io_sqe, file_descriptor_->borrow(), POLLIN);
        }

        void handle_operation_result(int32_t result, bool more) override {
            (void)more;

            result_ = result;
        }

        void handle_cancel_result(int32_t result, bool more) override {
            assert(!more);

            if (result >= 0) {
                result_ = -ECANCELED;
            }
        }

    private:
        Ref<FileDescriptor> file_descriptor_;
        int32_t result_;
    };

}
