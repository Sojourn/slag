#include <iostream>
#include "slag/slag.h"

using namespace slag;

class TestResource : public Resource {
public:
    using Resource::start_nop_operation;

    void handle_operation_complete(Operation& operation) override {
        (void)operation;
        info("Operation complete");
        local_event_loop().stop();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    TestResource test_resource;
    test_resource.start_nop_operation(nullptr);
    event_loop.run();
    return 0;
}
