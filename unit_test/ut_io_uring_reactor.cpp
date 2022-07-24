#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

class TestResource : public Resource {
public:
    using Resource::start_nop_operation;

    void handle_operation_complete(Operation&) override {
        local_event_loop().stop();
    }
};

TEST_CASE("io_uring reactor startup and shutdown", "[IOURingReactor]") {
    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    SECTION("nop") {
        TestResource resource;
        (void)resource.start_nop_operation(nullptr);

        event_loop.run();
    }
}
