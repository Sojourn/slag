#pragma once

#include "../../generated/record.h"
#include "message.h"

namespace slag {

    template<RecordType type>
    void decode(Record<type>& record, MessageReader& reader);

}
