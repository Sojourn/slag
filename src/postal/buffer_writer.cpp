#include "slag/postal/buffer_writer.h"
#include "slag/core/domain.h"
#include <stdexcept>
#include <cstring>
#include <cassert>

namespace slag {

    BufferWriter::BufferWriter(BufferAllocator& allocator)
        : allocator_{allocator}
        , handle_{make_handle(allocator.allocate_buffer())}
        , entry_{nation().buffer_ledger().get_national_entry(handle_.descriptor())}
    {
    }

    void BufferWriter::write(std::span<const std::byte> data) {
        if (!handle_) {
            throw std::runtime_error("Buffer has already been published");
        }

        while (!data.empty()) {
            if (storage_.empty()) {
                storage_ = allocator_.allocate_buffer_segment(
                    handle_.descriptor(),
                    data.size_bytes()
                );
            }

            size_t count = std::min(data.size_bytes(), storage_.size_bytes());
            memcpy(storage_.data(), data.data(), count);
            entry_.size += count;

            storage_ = storage_.subspan(count);
            data = data.subspan(count);
        }
    }

    BufferHandle BufferWriter::publish() {
        return std::move(handle_);
    }

}
