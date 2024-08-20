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

class TestTask : public Task {
public:
    TestTask()
        : print_("Hello, world!")
    {
        runnable_event_.set();
    }

    Event& runnable_event() override final {
        if (op_) {
            return op_->complete_event();
        }

        return runnable_event_;
    }

    void run() override final {
        if (!op_) {
            get_context().event_loop().schedule(print_);
            op_ = get_context().event_loop().start_operation<NopOperation>();
            return;
        }

        get_context().event_loop().stop();
        set_success(true);
    }

private:
    Ptr<NopOperation> op_;
    PrintTask         print_;
    Event             runnable_event_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Application application(argc, argv);
    application.spawn_thread<TestTask>();
    return application.run();
}
