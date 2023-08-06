#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/bit_set.h"
#include "slag/postal/types.h"

namespace slag::postal {

    template<DomainType domain_type>
    struct DomainCensus;

    template<>
    struct DomainCensus<DomainType::REGION> {
        BitSet                         buffer_ownership_changes;
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
