#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"

namespace slag::postal {

    template<DomainType domain_type>
    struct DomainConfig;

    template<>
    struct DomainConfig<DomainType::REGION> {
        size_t index        = 0;
        size_t buffer_count = 0;
    };

    template<>
    struct DomainConfig<DomainType::NATION> {
        size_t index                 = 0;
        size_t buffer_count          = 0;
        size_t region_count          = 0;
        size_t parcel_queue_capacity = 0;
    };

    template<>
    struct DomainConfig<DomainType::EMPIRE> {
        size_t index = 0;
    };

}
