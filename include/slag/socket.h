#pragma once

#include <utility>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/address.h"
#include "slag/future.h"
#include "slag/platform.h"
#include "slag/resource.h"
#include "slag/coroutine.h"
#include "slag/operation.h"
#include "slag/file_descriptor.h"
#include "slag/byte_stream.h"

namespace slag {

    class Socket : public Resource {
    public:
        Coroutine<Result<void>> open(sa_family_t family, int type, int protocol=Platform::DEFAULT_SOCKET_PROTOCOL);
        Coroutine<Result<void>> bind(const Address& address);
        Coroutine<Result<void>> listen(int backlog=Platform::DEFAULT_LISTEN_BACKLOG);
        Coroutine<std::pair<Socket, Address>> accept();
        Coroutine<Result<void>> send(std::span<const std::byte> data);

    private:
        void handle_operation_complete(Operation& operation) override;

    private:
        ByteStream             tx_stream_;
        ByteStream             rx_stream_;
        std::vector<std::byte> rx_buffer_;
    };

}
