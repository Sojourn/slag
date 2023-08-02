#pragma once

#include <compare>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace slag::postal {

    class Envelope;

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

    constexpr size_t BUFFER_CAPACITY_STRIDE = 64;
    constexpr size_t BUFFER_CAPACITY_SHIFT  = 6;
    constexpr size_t BUFFER_CAPACITY_MIN    = 1 << (BUFFER_CAPACITY_SHIFT +  0);               // 64B
    constexpr size_t BUFFER_CAPACITY_MAX    = 1 << (BUFFER_CAPACITY_SHIFT + sizeof(uint16_t)); // 4MB

    // TODO: constructor, operator bool, comparison operators, hashing
    // TODO: think about reuse within an epoch
    struct BufferIdentity {
        uint32_t index : 31;
        uint32_t valid : 1;
    };

    // TODO: constructor
    struct BufferProperties {
        uint16_t scaled_capacity;

        uint8_t frozen : 1; // buffer contents cannot be changed
        uint8_t shared : 1; // buffer is locally reference counted
        uint8_t global : 1; // buffer has been shared with other threads

        // uint8_t doomed : 1; // set by the table when it can be deleted
        // uint8_t frozen : 1;
        // uint8_t locked : 1; // mlocked?
        // uint8_t pinned : 1;
        // uint8_t hashed : 1; // is there a hash value for this?
        // uint8_t policy : 1;
        // uint8_t remote : 1; // another thread owns the buffer
        // uint8_t active : 1;
        // uint8_t parity : 1;
        // uint8_t public : 1;

        // user data
        // uint8_t label;
    };

    struct BufferDescriptor {
        BufferIdentity   identity;
        BufferProperties properties;
    };

    struct PostArea {
        uint16_t service = 0; // which postal service (one per process)
        uint16_t office = 0;  // which post office (one per thread)

        constexpr auto operator<=>(const PostArea&) const = default;
    };

    struct PostCode : PostArea {
        uint32_t number = 0; // local post-box number

        constexpr auto operator<=>(const PostCode&) const = default;
    };

    struct PostageStamp {
        uint64_t sequence = 0; // unique per-PostOffice
        // uint64_t purchased_value : ?;
        // uint64_t depreciated_value : ?;
        // uint64_t time_to_live : 5;
        // uint64_t acknowlege   : 1;
        // uint64_t delivered    : 1;
        // uint64_t returning    : 1;
    };

    static_assert(sizeof(BufferIdentity) == sizeof(uint32_t));
    static_assert(sizeof(BufferProperties) == sizeof(uint32_t));
    static_assert(sizeof(BufferDescriptor) == sizeof(uint64_t));

}
