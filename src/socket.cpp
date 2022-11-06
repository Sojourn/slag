#include "slag/socket.h"
#include "slag/platform.h"
#include "slag/fiber.h"
#include "slag/logging.h"
#include <cassert>

slag::Coroutine<slag::Result<void>> slag::Socket::open(sa_family_t family, int type, int protocol) {
    Result<int> file_descriptor_result = local_platform().socket(static_cast<int>(family), type, protocol);
    if (file_descriptor_result.has_error()) {
        co_return {file_descriptor_result.error()};
    }

    FileDescriptor file_descriptor{file_descriptor_result.value()};
    assert(file_descriptor);

    // FIXME: think more about how to set socket options
    static constexpr bool REUSE_ADDRESS = true;
    static constexpr bool REUSE_PORT = false; // allows multiple bindings

    if (REUSE_ADDRESS) {
        int value = 1;
        socklen_t value_length = sizeof(value);
        if (::setsockopt(file_descriptor.borrow(), SOL_SOCKET, SO_REUSEADDR, &value, value_length) < 0) {
            make_system_error().raise("Failed to set SO_REUSEADDR socket option");
        }
    }
    if (REUSE_PORT) {
        int value = 1;
        socklen_t value_length = sizeof(value);
        if (::setsockopt(file_descriptor.borrow(), SOL_SOCKET, SO_REUSEPORT, &value, value_length) < 0) {
            make_system_error().raise("Failed to set SO_REUSEPORT socket option");
        }
    }

    Operation& operation = start_assign_operation(nullptr, std::move(file_descriptor));
    Future<void> future = operation.parameters<OperationType::ASSIGN>().result.get_future();
    co_await future;
    co_return {};
}

slag::Coroutine<slag::Result<void>> slag::Socket::bind(const Address& address) {
    Operation& operation = start_bind_operation(nullptr, address);

    Future<void> future = operation.parameters<OperationType::BIND>().result.get_future();
    co_await future;
    co_return {};
}

slag::Coroutine<slag::Result<void>> slag::Socket::listen(int backlog) {
    Operation& operation = start_listen_operation(nullptr, backlog);

    Future<void> future = operation.parameters<OperationType::LISTEN>().result.get_future();
    co_await future;
    co_return {};
}

slag::Coroutine<std::pair<slag::Socket, slag::Address>> slag::Socket::accept() {
    Operation& accept_operation = start_accept_operation(nullptr);

    auto&& accept_future = accept_operation.parameters<OperationType::ACCEPT>().result.get_future();
    auto&& [file_descriptor, address] = co_await accept_future;

    slag::Socket socket;
    Operation& assign_operation = socket.start_assign_operation(nullptr, std::move(file_descriptor));
    Future<void> assign_future = assign_operation.parameters<OperationType::ASSIGN>().result.get_future();
    co_await assign_future;

    co_return std::make_pair(std::move(socket), address);
}

slag::Coroutine<size_t> slag::Socket::send(BufferSlice buffer_slice) {
    Operation& operation = start_send_operation(nullptr, std::move(buffer_slice));
    auto&& future = operation.parameters<OperationType::SEND>().result.get_future();
    co_return (co_await future);
}

void slag::Socket::handle_operation_complete(Operation& operation) {
    (void)operation;
}
