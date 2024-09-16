#include <iostream>

#include "mantle/mantle.h"
#include "slag/slag.h"

using namespace slag;

struct Worker : ProtoTask {
    Worker() {
        throw std::runtime_error("Error!");
    }

    void run() override final {
        SLAG_PT_BEGIN();
        {
            // Nop.
        }
        SLAG_PT_END();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    RuntimeConfig config;

    // Four threads in a ring.
    config.thread_topology.add_edge({0, 1});
    config.thread_topology.add_edge({1, 2});
    config.thread_topology.add_edge({2, 3});
    config.thread_topology.add_edge({3, 0});

    try {
        Runtime runtime(config);
        for (ThreadIndex thread_index = 0; thread_index < 4; ++thread_index) {
            runtime.spawn_thread<Worker>(ThreadConfig {
                .index          = thread_index,
                .name           = "worker",
                .cpu_affinities = std::nullopt,
            });
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Caught: " << ex.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
    