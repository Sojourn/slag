#include <iostream>
#include "slag/stack.h"
#include "slag/postal/task.h"
#include "slag/postal/proto_task.h"
#include "slag/postal/reactor.h"
#include "slag/postal/pollable.h"
#include "slag/postal/executor.h"
#include "slag/postal/domain.h"
#include "slag/postal/driver.h"
#include "slag/postal/service.h"
#include "slag/postal/services/system_service.h"
#include "slag/postal/services/memory_service.h"

using namespace slag;
using namespace slag::postal;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Empire::Config empire_config;
    empire_config.index = 0;
    Empire empire_{empire_config};

    Nation::Config nation_config;
    nation_config.index                 = 0;
    nation_config.buffer_count          = 16 * 1024 + 1;
    nation_config.region_count          = 1;
    nation_config.parcel_queue_capacity = 512;
    Nation nation_{nation_config};

    Region::Config region_config;
    region_config.index = 0;
    region_config.buffer_range = std::make_pair(0, nation_config.buffer_count);
    Region regino_{region_config};

    Driver<MemoryService, SystemService> driver;
    driver.run();

    return 0;
}
