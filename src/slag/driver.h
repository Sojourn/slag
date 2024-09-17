#pragma once

#include "slag/driver/shutdown_driver.h"
#include "slag/driver/region_driver.h"
#include "slag/driver/router_driver.h"

namespace slag {

    class EventLoop;

    class Drivers {
    public:
        explicit Drivers(EventLoop& event_loop)
            : shutdown_driver_(event_loop)
            , region_driver_(event_loop)
            , router_driver_(event_loop)
        {
        }

    private:
        ShutdownDriver shutdown_driver_;
        RegionDriver   region_driver_;
        RouterDriver   router_driver_;
    };

}
