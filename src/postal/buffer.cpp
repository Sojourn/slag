#include "slag/postal/buffer.h"
#include "slag/postal/buffer_table.h"

namespace slag::postal {

    BufferHandle::BufferHandle(BufferDescriptor descriptor)
        : descriptor_{descriptor}
    {
        if (is_shared()) {
            table().attach_shared(*this);
        }
        else {
            table().attach_unique(*this);
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
            table().reattach_unique(*this);
        }
    }

    BufferHandle& BufferHandle::operator=(BufferHandle&& rhs) {
        if (this != &rhs) {
            reset();

            std::swap(descriptor_, rhs.descriptor_);

            if (is_valid() && !is_shared()) {
                table().reattach_unique(*this);
            }
        }

        return *this;
    }

    BufferHandle::operator bool() const {
        return is_valid();
    }

    bool BufferHandle::is_valid() const {
        return descriptor_.identity.valid;
    }

    bool BufferHandle::is_shared() const {
        return descriptor_.properties.shared;
    }

    bool BufferHandle::is_global() const {
        return descriptor_.properties.global;
    }

    bool BufferHandle::is_frozen() const {
        return descriptor_.properties.frozen;
    }

    const BufferIdentity& BufferHandle::identity() const {
        return descriptor_.identity;
    }

    const BufferProperties& BufferHandle::properties() const {
        return descriptor_.properties;
    }

    const BufferDescriptor& BufferHandle::descriptor() const {
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
        if (!is_frozen()) {
            assert(false);
            return clone(); // copy-on-write
        }

        if (!is_shared()) {
            table().set_shared(*this);

            assert(is_shared()); // post-condition of BufferTable::set_shared
        }

        return BufferHandle(descriptor_);
    }

    void BufferHandle::reset() {
        if (!is_valid()) {
            return;
        }

        if (is_shared()) {
            table().detach_shared(*this);
        }
        else {
            table().detach_unique(*this);
        }

        descriptor_ = BufferDescriptor{};
    }

    BufferTable& BufferHandle::table() {
        abort(); // TODO: thread_local accessor
    }

    uint16_t to_scaled_capacity(size_t capacity) {
        return static_cast<uint16_t>(capacity - BUFFER_CAPACITY_STRIDE) / BUFFER_CAPACITY_STRIDE;
    }

    size_t from_scaled_capacity(uint16_t scaled_capacity) {
        return (static_cast<size_t>(scaled_capacity) * BUFFER_CAPACITY_STRIDE) + BUFFER_CAPACITY_STRIDE;
    }

}
