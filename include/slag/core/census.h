#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/collection/bit_set.h"
#include "slag/collection/spsc_queue.h"
#include "slag/postal/types.h"

namespace slag {

    template<DomainType domain_type>
    struct DomainCensus;

    template<>
    struct DomainCensus<DomainType::REGION> {
        BitSet                         buffer_reference_changes;
        std::vector<SpscQueueSequence> import_sequences;
        std::vector<SpscQueueSequence> export_sequences;
    };

    template<>
    struct DomainCensus<DomainType::NATION> {
        std::vector<DomainCensus<DomainType::REGION>> regions;
    };

    template<>
    struct DomainCensus<DomainType::EMPIRE> {
        std::vector<DomainCensus<DomainType::NATION>> nations;
    };

}
