#include "catch.hpp"
#include "slag/slag.h"
#include "slag/transform.h"
#include <limits>

using namespace slag;

TEST_CASE("transform") {
    SECTION("zig-zag")

    for (int64_t i = -100; i <= 100; ++i) {
        CHECK(decode_zig_zag(encode_zig_zag(i)) == i);
    }

    CHECK(decode_zig_zag(encode_zig_zag(std::numeric_limits<int64_t>::max())) == std::numeric_limits<int64_t>::max());
    CHECK(decode_zig_zag(encode_zig_zag(std::numeric_limits<int64_t>::min())) == std::numeric_limits<int64_t>::min());
}
