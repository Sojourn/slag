#pragma once

#include "message.h"
#include "../../generated/record.h"

namespace slag {

    template<RecordType type>
    void decode(Record<type>& record, MessageReader& reader);

    template<typename Handler>
    void decode(Handler&& handler, MessageReader& reader);

}

#include "record_decoder.hpp"
