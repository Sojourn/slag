#pragma once

#include <sys/mman.h>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cassert>

namespace slag {
    class stream_buffer {
    public:
        stream_buffer(size_t minimum_capacity)
            : file_descriptor_{-1}
        {
            if (minimum_capacity > (1ull << 30)) {
                throw std::bad_alloc;
            }

            size_t capacity = 4096;
            while (capacity < minimum_capacity) {
                capacity *= 2;
            }

            file_descriptor_ = ::memfd_create("stream_buffer", 0);
            if (file_descriptor_ < 0) {
                throw std::runtime_error("Failed to create memfd for stream buffer");
            }

            int rc = 0;
            do {
                rc = ::fallocate(file_descriptor_, 0, 0, static_cast<off_t>(capacity));
            } while ((rc < 0) && (errno == EINTR));
            if (rc < 0) {
                rc = ::close(file_descriptor_);
                assert(rc >= 0);
                throw std::runtime_error("Failed to ftruncate memfd for stream buffer");
            }

            void* lower_address = ::mmap(
                nullptr,
                capacity * 2,
                PROT_READ|PROT_WRITE,
                MAP_SHARED,
                file_descriptor_,
                0
            );
            if (lower_address == MAP_FAILED) {
                rc = ::close(file_descriptor_);
                assert(rc >= 0);
                throw std::runtime_error("Failed to mmap memfd for stream buffer (first)");
            }

            data_ = std::span {
                reinterpret_cast<std::byte*>(lower_address),
                capacity
            };

            void* upper_address = ::mmap(
                data_.data() + data_.size_bytes(),
                capacity * 2,
                PROT_READ|PROT_WRITE,
                MAP_SHARED|MAP_FIXED,
                file_descriptor_,
                0
            );
            if (upper_address == MAP_FAILED) {
                int rc = 0;
                do {
                    rc = ::munmap(data_.data(), data_.size_bytes() * 2);
                } while ((rc < 0) && (errno == EINTR));
                assert(rc >= 0);

                rc = ::close(file_descriptor_);
                assert(rc >= 0);
                throw std::runtime_error("Failed to mmap memfd for stream buffer (first)");
            }
        }

        ~stream_buffer() {
            int rc = 0;
            do {
                rc = ::munmap(data_.data(), data_.size_bytes() * 2);
            } while ((rc < 0) && (errno == EINTR));
            assert(rc >= 0);

            int rc = ::close(file_descriptor_);
            assert(rc >= 0);
        }

        size_t size() const {
            return data_.size();
        }

        std::span<std::byte> make_span(size_t offset, size_t length) {
            assert(length <= data_.size());
            return std::span {
                data_.data() + (offset & (data_.size() - 1)),
                length
            };
        }

        std::span<const std::byte> make_span(size_t offset, size_t length) const {
            assert(length <= data_.size());
            return std::span {
                data_.data() + (offset & (data_.size() - 1)),
                length
            };
        }

    private:
        std::span<std::byte> data_;
        int                  file_descriptor_;
    };
}
