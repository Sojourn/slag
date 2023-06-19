#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

TEST_CASE("Record") {
    SECTION("Encoding and Decoding") {
        TestStruct src_record, dst_record;

        src_record.a = {{1, 2, 3}};
        src_record.b = true;
        src_record.c = "double trouble";
        src_record.d = std::make_tuple(std::string{"zero"}, std::vector<uint8_t>{{4, 5, 6}});
        {
            auto&& map = src_record.e.emplace<std::unordered_map<int8_t, int8_t>>();
            map[0] = 0;
            map[1] = 1;
            map[2] = 2;
        }
        src_record.f.push_back(static_cast<std::byte>(13));
        src_record.f.push_back(static_cast<std::byte>(14));
        src_record.f.push_back(static_cast<std::byte>(15));
        src_record.f.push_back(static_cast<std::byte>(16));

        Message message;
        {
            MessageWriter writer{message};
            encode(src_record, writer);

            MessageReader reader{message};
            decode(dst_record, reader);
        }

        CHECK(src_record.a == dst_record.a);
        CHECK(src_record.b == dst_record.b);
        CHECK(src_record.c == dst_record.c);
        CHECK(src_record.d == dst_record.d);
        CHECK(src_record.e == dst_record.e);
        CHECK(src_record.f == dst_record.f);
    }
}
