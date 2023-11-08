#include "slag/memory/buffer_handle.h"
#include <utility>
#include <cassert>

namespace slag {

    static MemoryServiceInterface& get_memory_service(BufferDescriptor descriptor) {
        return get_memory_service(
            get_service_registry(descriptor.worker_index)
        );
    }

    BufferHandle::BufferHandle(BufferDescriptor descriptor);
        : descriptor_{descriptor}
    {
        // Whoever is constructing us is responsible for our initial reference.
    }

    BufferHandle::~BufferHandle() {
        reset();
    }

    BufferHandle::BufferHandle(BufferHandle&& other)
        : memory_service_{nullptr}
    {
        std::swap(descriptor_, other.descriptor_);
    }

    BufferHandle::BufferHandle& operator=(BufferHandle&& that) {
        if (this != &that) {
            reset();

            std::swap(descriptor_, that.descriptor_);
        }

        return *this;
    }

    BufferHandle::operator bool() const {
        return descriptor_.buffer_index >= 0;
    }

    BufferDescriptor BufferHandle::descriptor() const {
        return descriptor_;
    }

    BufferHandle BufferHandle::share() {
        if (*this) {
            increment_reference_count();
        }

        return {
            descriptor_
        };
    }

    void BufferHandle::reset() {
        if (*this) {
            decrement_reference_count();
        }

        descriptor_ = BufferDescriptor{};
    }

    void BufferHandle::increment_reference_count() {
        assert(*this);

        get_memory_service(descriptor_).increment_reference_count(descriptor_);
    }

    void BufferHandle::decrement_reference_count() {
        assert(*this);

        get_memory_service(descriptor_).decrement_reference_count(descriptor_);
    }

}
