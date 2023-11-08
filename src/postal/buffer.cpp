#include "slag/postal/buffer.h"
#include "slag/core/domain.h"
#include <cstring>
#include <cassert>

namespace slag {

    BufferHandle::BufferHandle(BufferDescriptor descriptor)
        : descriptor_{descriptor}
    {
        // Whoever is constructing us is responsible for our initial reference.
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
            reset();

            std::swap(descriptor_, that.descriptor_);
        }

        return *this;
    }

    BufferHandle::operator bool() const {
        return descriptor_.index > 0;
    }

    BufferDescriptor BufferHandle::descriptor() const {
        return descriptor_;
    }

    size_t BufferHandle::size() const {
        if (!*this) {
            return 0;
        }

        return national_entry().size;
    }

    NationalBufferLedgerEntry& BufferHandle::national_entry() {
        assert(*this);
        return nation().buffer_ledger().get_national_entry(descriptor_);
    }

    const NationalBufferLedgerEntry& BufferHandle::national_entry() const {
        assert(*this);
        return nation().buffer_ledger().get_national_entry(descriptor_);
    }

    RegionalBufferLedgerEntry& BufferHandle::regional_entry() {
        assert(*this);
        return region().buffer_ledger().get_regional_entry(descriptor_);
    }

    const RegionalBufferLedgerEntry& BufferHandle::regional_entry() const {
        assert(*this);
        return region().buffer_ledger().get_regional_entry(descriptor_);
    }

    BufferHandle BufferHandle::share() {
        if (*this) {
            increment_reference_count();
        }

        return BufferHandle {
            descriptor_
        };
    }

    void BufferHandle::reset() {
        if (*this) {
            decrement_reference_count();
            memset(&descriptor_, 0, sizeof(descriptor_));
        }
    }

    void BufferHandle::increment_reference_count() {
        region().buffer_ledger().increment_reference_count(descriptor_);
    }

    void BufferHandle::decrement_reference_count() {
        region().buffer_ledger().decrement_reference_count(descriptor_);
    }

}
