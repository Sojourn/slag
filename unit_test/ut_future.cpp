#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

TEST_CASE("Future") {
    auto&& [future, promise] = make_future<int>();

    CHECK(!future.has_result());
    CHECK(!future.has_value());
    CHECK(!future.has_error());

    promise.set_value(13);
    CHECK(future.has_result());
    CHECK(future.has_value());
    CHECK(!future.has_error());
    CHECK(future.value() == 13);
    CHECK(future.get() == 13);
}
