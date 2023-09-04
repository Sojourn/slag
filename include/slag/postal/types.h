#pragma once

#include <compare>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace slag::postal {

    // Length-constrained names, for fun:
    //   Budget
    //   Leader
    //   Worker
    //   Policy
    //   Corpus
    //   Bureau
    //   Agency
    //   Broker
    //   Dealer
    //   Module
    //   Survey
    //   Mirror
    //   Portal
    //   Fabric
    //   Runway
    //   Stream
    //   Layout
    //   Format
    //   System
    //   Device
    //   Devise
    //   Tenure
    //   County

    // mirror mirror on the wall, who is the most contiguous buffer of them all

    class Reactor;

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

    enum class DomainType {
        EMPIRE,
        NATION,
        REGION,
    };

    class Domain;
    class Empire;
    class Nation;
    class Region;

    // this tells us what thread to route an envelope to
    // TODO: rename to PostalArea
    struct PostArea {
        uint16_t nation = 0; // which postal service (one per process)
        uint16_t region = 0; // which post office (one per thread)

        constexpr auto operator<=>(const PostArea&) const = default;
    };

    // this tells us what PostBox on that thread
    // TODO: rename to PostalAddress
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
