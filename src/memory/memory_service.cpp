#include "slag/memory/memory_service.h"

namespace slag {

    MemoryService::MemoryService() {
    }

    void MemoryService::start_service() {
    }

    void MemoryService::stop_service() {
    }

    std::span<std::byte> MemoryService::allocate_block() {
        return {};
    }

    void MemoryService::deallocate_block(std::span<std::byte> block) {
        (void)block;
    }

}
