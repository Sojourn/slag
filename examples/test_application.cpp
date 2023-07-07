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

    return application.run();
}
