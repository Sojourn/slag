#pragma once

#include <cassert>
#include "slag/object.h"
#include "slag/core/proto_task.h"
#include "slag/system/reactor.h"
#include "slag/system/file_descriptor.h"
#include "slag/system/operation_factory.h"
#include "slag/system/operations/poll_multishot_operation.h"

namespace slag {

    class RegionDriver final : public ProtoTask {
    public:
        RegionDriver(Region& region, Reactor& reactor)
            : region_(region)
            , reactor_(reactor)
            , region_fd_(
                make_file_descriptor(region_.file_descriptor(), FileDescriptor::Ownership::BORROWED)
            )
        {
        }

        void run() override {
            SLAG_PT_BEGIN();

            while (true) {
                if (!poll_) {
                    poll_ = start_poll_multishot_operation(region_fd_);
                }

                SLAG_PT_WAIT_READABLE(*poll_);
                if (poll_->result() < 0) {
                    poll_->cancel();
                }
                else {
                    assert(poll_->result() & POLLIN); // Just curious--spurious steps are fine.

                    constexpr bool non_blocking = true;
                    region_.step(non_blocking);
                }

                if (poll_->is_complete()) {
                    poll_.reset();
                }
                else {
                    // The operation is still active.
                }
            }

            SLAG_PT_END();
        }

    private:
        Region& region_;
        Reactor& reactor_;
        Ref<FileDescriptor> region_fd_;
        Ptr<PollMultishotOperation> poll_;
    };

}
