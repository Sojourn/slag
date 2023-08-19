#include "slag/postal/buffer.h"
#include "slag/postal/domain.h"
#include <cstring>
#include <cassert>

namespace slag::postal {

    BufferHandle::BufferHandle() {
        memset(&descriptor_, 0, sizeof(descriptor_));
    }

    BufferHandle::~BufferHandle() {
        reset();
    }

    BufferHandle::BufferHandle(BufferHandle&& other)
        : BufferHandle{}
    {
        std::swap(descriptor_, other.descriptor_);
    }

    BufferHandle& BufferHandle::operator=(BufferHandle&& that) {
        if (this != &that) {
            // TODO
        }

        return *this;
    }

    BufferHandle::operator bool() const {
        return descriptor_.index > 0;
    }

    void BufferHandle::reset() {
        // TODO
    }

}
