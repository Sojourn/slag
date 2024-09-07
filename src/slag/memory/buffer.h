#pragma once

#include <span>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstddef>

#include "slag/object.h"
#include "slag/resource.h"

namespace slag {

    // TODO: Use private memory mappings (huge-pages) as a backing store.
    //
    template<>
    class Resource<ResourceType::BUFFER> : public Object {
    public:
        explicit Resource(const size_t capacity)
            : Object(static_cast<ObjectGroup>(ResourceType::BUFFER))
            , storage_(capacity)
        {
        }

        std::span<std::byte> storage() {
            return {
                storage_.data(),
                storage_.size(),
            };
        }

        std::span<const std::byte> storage() const {
            return {
                storage_.data(),
                storage_.size(),
            };
        }

    private:
        std::vector<std::byte> storage_;
    };

    class BufferSlice {
    public:
        explicit BufferSlice(const Ref<Buffer>& buffer)
            : buffer_(buffer)
            , selection_(buffer_->storage())
        {
        }

        BufferSlice(const Ref<Buffer>& buffer, std::span<std::byte> selection)
            : buffer_(buffer)
            , selection_(selection)
        {
            assert(buffer_->storage().begin() <= selection_.begin());
            assert(selection_.end() <= buffer_->storage().end());
        }

        std::span<std::byte> selection() {
            return selection_;
        }

        std::span<const std::byte> selection() const {
            return selection_;
        }

    private:
        Ref<Buffer> buffer_;
        std::span<std::byte> selection_;
    };

}
