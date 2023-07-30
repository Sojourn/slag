#pragma once

#include <compare>
#include <cstdint>
#include <cstddef>

namespace slag::postal {

    // send/receive queue of envelopes
    class PostBox;

    // per-thread datastructures
    class PostOffice;

    // per-process datastructures
    class PostalService;

    // in charge of a PostOffice (one per thread)
    class PostMaster;

    // in charge of the PostalService in a region
    class PostMasterGeneral;

    constexpr size_t BUFFER_GROUP_COUNT     = 8;
    constexpr size_t BUFFER_CAPACITY_STRIDE = 64;
    constexpr size_t BUFFER_CAPACITY_SHIFT  = 6;
    constexpr size_t BUFFER_CAPACITY_MIN    = 1 << (BUFFER_CAPACITY_SHIFT +  0);               // 64B
    constexpr size_t BUFFER_CAPACITY_MAX    = 1 << (BUFFER_CAPACITY_SHIFT + sizeof(uint16_t)); // 4MB

    struct BufferIdentity {
        uint32_t index : 28;
        uint32_t group : 3;  // 8 groups
        uint32_t valid : 1;  // does this point to a buffer?
    };

    struct BufferProperties {
        uint16_t scaled_capacity; // = capacity / BUFFER_CAPACITY_STRIDE

        // flags
        uint8_t doomed : 1;
        uint8_t frozen : 1;
        uint8_t locked : 1; // mlocked?
        uint8_t pinned : 1;
        uint8_t shared : 1; // reference counted
        uint8_t global : 1; // shared & global -> atomic reference counted
        uint8_t hashed : 1; // is there a hash value for this?
        uint8_t policy : 1;
        // uint8_t parity : 1;

        // user data
        uint8_t label;
    };

    struct BufferDescriptor {
        BufferIdentity   identity;
        BufferProperties properties;
    };

    struct PostArea {
        uint16_t service; // which process (should have the same ordering as IP addresses)
        uint16_t office;  // which worker thread in the process

        constexpr inline auto operator<=>(const PostArea&) const = default;
    };

    struct PostCode : PostArea {
        uint32_t number; // local post-box number

        constexpr inline auto operator<=>(const PostCode&) const = default;
    };

    struct Stamp {
        uint64_t sequence     : 56; // wraps
        uint64_t time_to_live : 5;
        uint64_t acknowlege   : 1;
        uint64_t delivered    : 1;
        uint64_t returning    : 1;
    };

    struct Parcel {
        PostCode         to;
        PostCode         from;
        Stamp            stamp;
        BufferDescriptor contents;
    };

    static_assert(sizeof(BufferIdentity) == sizeof(uint32_t));
    static_assert(sizeof(BufferProperties) == sizeof(uint32_t));
    static_assert(sizeof(BufferDescriptor) == sizeof(uint64_t));

}
