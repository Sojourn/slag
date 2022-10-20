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

    Result<void> result = future.result();
    co_return result;
}

slag::Coroutine<slag::Result<void>> slag::Socket::bind(const Address& address) {
    Operation& operation = start_bind_operation(nullptr, address);

    Future<void> future = operation.parameters<OperationType::BIND>().result.get_future();
    co_await future;

    Result<void> result = future.result();
    co_return result;
}

slag::Coroutine<slag::Result<void>> slag::Socket::listen(int backlog) {
    Operation& operation = start_listen_operation(nullptr, backlog);

    Future<void> future = operation.parameters<OperationType::LISTEN>().result.get_future();
    co_await future;

    Result<void> result = future.result();
    co_return result;
}

void slag::Socket::handle_operation_complete(Operation& operation) {
    (void)operation;
}
