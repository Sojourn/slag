#include <iostream>

#include "slag/slag.h"

using namespace slag;
using namespace slag::postal;

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

    Empire empire_{0};
    Nation nation_{1};
    Region region_{2};

    std::cout << empire().identity() << std::endl;
    std::cout << nation().identity() << std::endl;
    std::cout << region().identity() << std::endl;

    return application.run();
}