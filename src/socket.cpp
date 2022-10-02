#include "slag/socket.h"
#include "slag/platform.h"
#include <cassert>

slag::Coroutine<slag::Result<void>> slag::Socket::open(sa_family_t family, int type, int protocol) {
    Result<int> file_descriptor_result = local_platform().socket(static_cast<int>(family), type, protocol);
    if (file_descriptor_result.has_error()) {
        co_return {file_descriptor_result.error()};
    }

    co_return {};
}

void slag::Socket::handle_operation_complete(Operation& operation) {
    (void)operation;
}
