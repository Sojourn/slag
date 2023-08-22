#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_ledger.h"

namespace slag::postal {

    class BufferReader {
    public:
        BufferReader(BufferHandle handle);

        size_t size() const;
        size_t tell() const;

        void seek(size_t position);
        void advance(size_t count);
        std::span<const std::byte> read(size_t count);

    private:
        BufferHandle                     handle_;
        const NationalBufferLedgerEntry& entry_;
        const BufferSegment*             segment_;
        std::span<const std::byte>       storage_;
        size_t                           position_;
    };

}
