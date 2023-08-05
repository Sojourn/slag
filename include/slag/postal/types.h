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

    // this tells us what thread to route an envelope to
    struct PostArea {
        uint16_t nation = 0; // which postal service (one per process)
        uint16_t region = 0; // which post office (one per thread)

        constexpr auto operator<=>(const PostArea&) const = default;
    };

    // this tells us what PostBox on that thread
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

}
