#include <iostream>

#include "mantle/mantle.h"
#include "slag/slag.h"

using namespace slag;

struct PrintTask final : ProtoTask {
    const char* message;

    explicit PrintTask(const char* message)
        : message(message)
    {
    }

    void run() override {
        SLAG_PT_BEGIN();

        std::cout << message << std::endl;

        SLAG_PT_END();
    }
};

struct TestTask : ProtoTask {
    Ptr<NopOperation>        op_;
    std::optional<PrintTask> print_;

    void run() override final {
        SLAG_PT_BEGIN();
        {
            op_ = start_nop_operation();
            SLAG_PT_WAIT_COMPLETE(*op_);

            print_.emplace("Hello, World!");
            SLAG_PT_WAIT_COMPLETE(*print_);
        }
        SLAG_PT_END();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    ThreadGraph graph;
    graph.add_edge({0, 1});
    graph.add_edge({1, 2});
    graph.add_edge({2, 3});
    graph.add_edge({3, 0});

    // ThreadRouteTable routes = build_thread_route_table(graph, 0);

    Runtime runtime(graph);
    runtime.spawn_thread<TestTask>(ThreadConfig {
        .index          = 0,
        .name           = "controller",
        .cpu_affinities = std::nullopt,
    });

    return EXIT_SUCCESS;
}
