#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

TEST_CASE("future errors", "[Future]") {
    SECTION("detached future") {
        Future<int> future;
        REQUIRE_THROWS_AS(future.result(), std::runtime_error);
    }

    SECTION("future not ready") {
        Promise<int> promise;
        Future<int> future{promise.get_future()};
        CHECK(future.result().error() == Error{ErrorCode::FUTURE_NOT_READY});
    }

    SECTION("future already retrieved") {
        {
            Promise<int> promise;
            Future<int> future = promise.get_future();
            REQUIRE_THROWS_AS(promise.get_future(), std::runtime_error);
        }
        {
            Promise<int> promise;
            (void)promise.get_future(); // discard the future
            REQUIRE_THROWS_AS(promise.get_future(), std::runtime_error);
        }
    }
}
