#pragma once

#include "slag/spsc_queue.h"
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"

namespace slag::postal {

    struct Parcel {
        PostCode         to;
        PostCode         from;
        PostageStamp     stamp;
        BufferDescriptor content;
    };
    static_assert(sizeof(Parcel) <= 64);

    using ParcelQueue         = SpscQueue<Parcel>;
    using ParcelQueueConsumer = SpscQueueConsumer<Parcel>;
    using ParcelQueueProducer = SpscQueueProducer<Parcel>;

}
