#pragma once

#include "../../generated/record.h"
#include "message.h"

namespace slag {

    template<RecordType type>
    void encode(const Record<type>& record, MessageWriter& writer);

}
