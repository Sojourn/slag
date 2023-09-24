#include <iostream>

#include "slag/slag.h"

using namespace slag;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    auto application_config = ApplicationConfig {
        .controller_thread = ControllerThreadConfig {
            .cpu_affinity = 0,
        },
        .worker_threads = {{
            WorkerThreadConfig {
                .cpu_affinity           = 1,
                .request_queue_capacity = 512,
                .reply_queue_capacity   = 512,
            },
            WorkerThreadConfig {
                .cpu_affinity           = 2,
                .request_queue_capacity = 512,
                .reply_queue_capacity   = 512,
            },
        }},
    };

    Application application {
        application_config
    };

    Queue<int> queue;
    queue.push_back(1);
    queue.push_back(2);
    queue.push_back(3);

    while (auto element = queue.pop_front()) {
        std::cout << *element << std::endl;
    }

    Empire::Config empire_config;
    empire_config.index = 0;

    Nation::Config nation_config;
    nation_config.index                 = 0;
    nation_config.buffer_count          = 1024;
    nation_config.region_count          = 1;
    nation_config.parcel_queue_capacity = 512;

    Region::Config region_config;
    region_config.index        = 0;
    region_config.buffer_range = std::make_pair(0, 1024);

    Empire empire_{empire_config};
    Nation nation_{nation_config};
    Region region_{region_config};

    std::cout << empire().index() << std::endl;
    std::cout << nation().index() << std::endl;
    std::cout << region().index() << std::endl;

    return application.run();
}
