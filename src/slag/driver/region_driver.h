#pragma once

#include "slag/object.h"
#include "slag/core.h"
#include "slag/system/reactor.h"
#include "slag/system/file_descriptor.h"
#include "slag/system/operation_factory.h"
#include "slag/system/operations/poll_multishot_operation.h"

#include <cassert>

namespace slag {

    class RegionDriver final : public ProtoTask {
    public:
        explicit RegionDriver(EventLoop& event_loop);

        void run() override;

    private:
        Region& region_;
        Reactor& reactor_;
        Ref<FileDescriptor> region_fd_;
        Ptr<PollMultishotOperation> poll_;
    };

}
