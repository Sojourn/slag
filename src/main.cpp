#include <iostream>

#include "mantle/mantle.h"
#include "slag/slag.h"
#include "slag/collections/intrusive_list.h"

using namespace slag;

class TestTask : public Task {
public:
    TestTask() {
        runnable_event_.set();
    }

    Event& runnable_event() override final {
        return runnable_event_;
    }

    void run() override final {
        get_context().event_loop().stop();
        set_success(true);
    }

private:
    Event runnable_event_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Application application(argc, argv);
    application.spawn_thread<TestTask>();
    return application.run();
}
