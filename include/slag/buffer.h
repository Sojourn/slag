#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/intrusive_list.h"

namespace slag {

    // Think about non-memory segments (for sendfile, etc.)
    // Think about how consumers/producers will interact wit this.
    // Think about how ownership should work between the event/loop and resource
    //
    // Buddy system allocator for buffers (and allow resizing)
    //
    struct BufferSegment {
        IntrusiveListNode    node;
        std::span<std::byte> data;
        uint32_t             head = 0;
        uint32_t             tail = 0;
    };

    // shared_ptr for ownership?
    class Buffer {
    public:
        [[nodiscard]] size_t size() const;

        [[nodiscard]] std::span<const std::byte> peek(std::optional<size_t> length = std::nullopt);
        [[nodiscard]] std::span<const std::byte> read(std::optional<size_t> length = std::nullopt);

        void append(std::span<const std::byte> data);
        void append(Buffer other);

    private:
        [[nodiscard]] BufferSegment& allocate_segment(size_t size_hint = 16 * 1024);
        void deallocate_segment(BufferSegment& segment);

    private:
        uint64_t producer_sequence_ = 0;
        uint64_t consumer_sequence_ = 0;
        IntrusiveList<BufferSegment, &BufferSegment::node> segments_;
    };

    // Buddy-system based buffer segment allocator. Should be able to start
    // with a pretty simple pool-based alloctor and add support for expanding/shrinking
    // segments later.
    //
    // should the buffer system be Future/Coroutine based?
    // - might need to use operations to register buffers
    // - could also use this to coordinate buffer sharing between threads
    //
    class BufferAllocator {
    public:
        [[nodiscard]] BufferSegment& allocate(size_t capacity_hint);
        void deallocate(BufferSegment& segment);

        // add at least this much capacity to the back of the segment
        [[nodiscard]] bool expand(BufferSegment& segment, size_t capacity_hint);

        // remove up to this much capacity from the front of the segment
        [[nodiscard]] bool shrink(BufferSegment& segment, size_t capacity_hint);

    private:
    };

}
