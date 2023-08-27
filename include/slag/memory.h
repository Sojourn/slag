#pragma once

#include <span>
#include <cstdint>
#include <cstddef>

namespace slag {

    // Quick and dirty way to make huge pages available:
    //   echo # > /proc/sys/vm/nr_hugepages
    std::span<std::byte> allocate_huge_pages(size_t size_bytes);
    void deallocate_huge_pages(std::span<std::byte> memory);

}
