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

    // TODO: make this more ergonomic
    {
        Operation& operation = start_assign_operation(nullptr, std::move(file_descriptor));
        Future<void> assign_result = operation.parameters<OperationType::ASSIGN>().result.get_future();

        co_await assign_result;
    }

    co_return {};
}

void slag::Socket::handle_operation_complete(Operation& operation) {
    (void)operation;
}
