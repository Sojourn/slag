#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/postal/domain.h"
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_ledger.h"

namespace slag::postal {

    class BufferWriter {
    public:
        explicit BufferWriter(BufferAllocator& allocator = region().buffer_allocator());

        void write(std::span<const std::byte> data);

        BufferHandle publish();

    private:
        BufferAllocator&           allocator_;
        BufferHandle               handle_;
        NationalBufferLedgerEntry& entry_;
        std::span<std::byte>       storage_;
    };

}
