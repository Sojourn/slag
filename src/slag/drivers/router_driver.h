#pragma once

#include "slag/object.h"
#include "slag/core.h"
#include "slag/bus.h"

#include <cassert>

namespace slag {

    class RouterDriver final : public ProtoTask {
    public:
        RouterDriver(Router& router);

    private:
        Router& router_;
    };

}
