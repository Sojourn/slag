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

void slag::Socket::handle_operation_complete(Operation& operation) {
    (void)operation;
}
