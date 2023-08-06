#pragma once

#include "slag/postal/types.h"
#include "slag/postal/buffer.h"

namespace slag::postal {

    struct Parcel {
        PostCode         to;
        PostCode         from;
        BufferDescriptor content;
    };

}
