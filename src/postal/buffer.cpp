#include "slag/postal/buffer.h"
#include <cassert>

namespace slag::postal {

    BufferDescriptor::BufferDescriptor() {
        memset(this, 0, sizeof(*this));
    }

    BufferHandle::BufferHandle(BufferDescriptor descriptor)
        : descriptor_{descriptor}
    {
        if (is_shared()) {
            // table().attach_shared(*this);
        }
        else {
            // table().attach_unique(*this);
        }
    }

    BufferHandle::~BufferHandle() {
        reset();
    }

    BufferHandle::BufferHandle(BufferHandle&& other)
        : descriptor_{other.descriptor_}
    {
        other.descriptor_ = BufferDescriptor{};

        if (is_valid() && !is_shared()) {
            // table().reattach_unique(*this);
        }
    }

    BufferHandle& BufferHandle::operator=(BufferHandle&& rhs) {
        if (this != &rhs) {
            reset();

            std::swap(descriptor_, rhs.descriptor_);

            if (is_valid() && !is_shared()) {
                // table().reattach_unique(*this);
            }
        }

        return *this;
    }

    BufferHandle::operator bool() const {
        return is_valid();
    }

    bool BufferHandle::is_valid() const {
        return static_cast<bool>(descriptor_);
    }

    bool BufferHandle::is_shared() const {
        return descriptor_.shared;
    }

    bool BufferHandle::is_global() const {
        return descriptor_.global;
    }

    BufferDescriptor BufferHandle::descriptor() const {
        return descriptor_;
    }

    // TODO: once we have allocation support
    // BufferHandle BufferHandle::clone() {
    //     if (!is_valid()) {
    //         return {}; // cloned an invalid handle
    //     }
    // }

    BufferHandle BufferHandle::share() {
        if (!is_valid()) {
            assert(false);
            return BufferHandle{};
        }

        if (!is_shared()) {
            // table().set_shared(*this);

            assert(is_shared()); // post-condition of BufferTable::set_shared
        }

        return BufferHandle(descriptor_);
    }

    void BufferHandle::reset() {
        if (!is_valid()) {
            return;
        }

        if (is_shared()) {
            // table().detach_shared(*this);
        }
        else {
            // table().detach_unique(*this);
        }

        descriptor_ = BufferDescriptor{};
    }

}
