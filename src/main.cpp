#include <iostream>
#include <string>

#include "mantle/mantle.h"
#include "slag/slag.h"

using namespace slag;

class Worker : public ProtoTask {
public:
    Worker()
        : channel_(std::to_string(get_thread().index()))
    {
    }

    void run() override final {
        SLAG_PT_BEGIN();
        {
            do {
                SLAG_PT_YIELD();
                target_ = channel_.query("0");
            } while (!target_);

            std::cout << get_thread().index() << " ready" << std::endl;

            timer_.set(std::chrono::milliseconds(100));
            while (!timer_.is_expired()) {
                std::cout << get_thread().index() << " yielding" << std::endl;
                SLAG_PT_YIELD();
            }
        }
        SLAG_PT_END();
    }

private:
    Channel channel_;
    std::optional<ChannelId> target_;
    BasicTimer timer_;
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
    