#pragma once

#include <span>
#include <stdexcept>
#include <cstdint>
#include <cstddef>

#include "slag/core/resource.h"
#include "slag/memory/allocator.h"

namespace slag {

    struct BufferSegmentStorage {
        std::span<std::byte> content;
        Allocator*           allocator = nullptr;
    };

    struct BufferSegment : BufferSegmentStorage {
        BufferSegment* next = nullptr;
    };

    // This is the cleanest design and gives us the best invariants.
    template<>
    class Resource<ResourceType::BUFFER> : public ResourceBase {
    public:
        explicit Resource()
            : ResourceBase(ResourceType::BUFFER)
            , size_(0)
            , tail_(&head_)
        {
        }

        size_t size() const {
            return size_;
        }

        BufferSegment& head() {
            return head_;
        }

        const BufferSegment& head() const {
            return head_;
        }

        BufferSegment& tail() {
            return *tail_;
        }

        const BufferSegment& tail() const {
            return *tail_;
        }

        // Conditionally need to allocate a segment. First one's free.
        // void append(BufferSegmentStorage storage, BufferSegmentAllocator& segment_allocator);

    private:
        size_t         size_;
        BufferSegment  head_;
        BufferSegment* tail_;
    };

    using Buffer       = Resource<ResourceType::BUFFER>;
    using BufferHandle = mantle::Handle<Buffer>;

    // TODO: Move this into a file and make it a class.
    struct BufferCursor {
        BufferHandle   buffer;
        size_t         buffer_offset = 0;

        BufferSegment* segment = nullptr;
        size_t         segment_offset = 0;
    };

}
