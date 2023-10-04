#pragma once

#include <sys/mman.h>
#include "slag/util.h"
#include "slag/layer.h"
#include "slag/logging.h"
#include "slag/core/service.h"
#include "slag/system/operation_factory.h"

namespace slag {

    class MemoryService : public ServiceInterface<ServiceType::MEMORY> {
    public:
        MemoryService();

        void start_service() override final;
        void stop_service() override final;

        std::span<std::byte> allocate_block() override final;
        void deallocate_block(std::span<std::byte> block) override final;
    };

}
