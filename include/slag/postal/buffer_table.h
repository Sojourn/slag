#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/bit_set.h"

namespace slag::postal {

    class BufferHandle;

    class BufferTable {
    public:
        union References {
            BufferHandle* handle; // used when unique
            size_t        count;  // used when shared
        };

        struct Record {
            std::span<std::byte> buffer;
            References           references;

            bool                 doomed = false;
            bool                 unused = false; // not referenced
            bool                 shared = false; // local reference counting
            bool                 global = false;
            bool                 frozen = false;

            // a flag to indicate if we own the buffer
            bool                 remote = false; // origin != local_area
            //                   import
            //                   export
        };

    public:
        // ???
        std::optional<uint32_t> allocate_index();
        void deallocate_index(uint32_t index);

    private:
        friend class BufferHandle;

        // branch in the handle instead of after dereferencing both us
        // and the record
        void attach_shared(BufferHandle& handle);
        void detach_shared(BufferHandle& handle);

        void attach_unique(BufferHandle& handle);
        void reattach_unique(BufferHandle& handle);
        void detach_unique(BufferHandle& handle);

        void set_shared(BufferHandle& handle);

    private:
        std::vector<Record>   records_;
        std::vector<uint32_t> tombstones_; // indexes of unused slots we control
    };

}
