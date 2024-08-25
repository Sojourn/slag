#include <iostream>

#include "mantle/mantle.h"
#include "slag/slag.h"
#include "slag/collections/intrusive_list.h"
#include "slag/thread_context.h"

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

    Runtime runtime(argc, argv);
    runtime.spawn_thread<TestTask>();

    return EXIT_SUCCESS;
}
