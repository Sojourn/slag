#include "slag/memory.h"
#include <sys/mman.h>
#include <stdexcept>
#include <cassert>

namespace slag {

    std::span<std::byte> allocate_huge_pages(size_t size_bytes) {
        void* address = mmap(
            NULL,
            size_bytes,
            PROT_READ|PROT_WRITE,
            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB,
            -1,
            0
        );
        if (address == MAP_FAILED) {
            throw std::runtime_error("Failed to allocate huge page");
        }

        return {
            reinterpret_cast<std::byte*>(address),
            size_bytes
        };
    }

    void deallocate_huge_pages(std::span<std::byte> memory) {
        int status = munmap(memory.data(), memory.size_bytes());
        assert(status >= 0);
    }

}