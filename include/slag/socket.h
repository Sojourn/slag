#pragma once

#include "slag/address.h"
#include "slag/future.h"
#include "slag/platform.h"
#include "slag/resource.h"
#include "slag/coroutine.h"
#include "slag/operation.h"
#include "slag/file_descriptor.h"

namespace slag {

    class Socket : public Resource {
    public:
        Coroutine<Result<void>> open(sa_family_t family, int type, int protocol=Platform::DEFAULT_SOCKET_PROTOCOL);
        Coroutine<Result<void>> bind(const Address& address);
        Coroutine<Result<void>> listen(int backlog=Platform::DEFAULT_LISTEN_BACKLOG);
        Coroutine<std::pair<Socket, Address>> accept();

    private:
        void handle_operation_complete(Operation& operation) override;
    };

}
