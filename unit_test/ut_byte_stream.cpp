#include "catch.hpp"
#include "slag/slag.h"
#include <vector>
#include <random>
#include <initializer_list>

using namespace slag;

TEST_CASE("reading_and_writing", "[ByteStream]") {
    ByteStream stream;

    std::vector<std::byte> data;
    auto make_data = [&](size_t count) {
        data.resize(count);
        for (size_t i = 0; i < count; ++i) {
            data[i] = static_cast<std::byte>(i);
        }

        return std::span<const std::byte>{data.data(), data.size()};
    };

    auto make_buffer = [&](size_t count) {
        auto data = make_data(count);
        return std::make_pair(data, make_handle<Buffer>(data));
    };

    SECTION("write") {
        stream.write(make_data(14));
        CHECK(stream.readable_byte_count() == 14);

        auto&& [wr_data, wr_buffer] = make_buffer(17);
        stream.write(wr_data, wr_buffer);
        CHECK(stream.readable_byte_count() == (14 + 17));
    }

    SECTION("read") {
        stream.write(make_data(14));
        stream.write(make_data(17));

        {
            auto&& [rd_data, rd_buffer] = stream.read_stable(7);
            CHECK(rd_data.size_bytes() == 7);
            CHECK(rd_buffer);
        }
        {
            auto&& [rd_data, rd_buffer] = stream.read_stable(9);
            CHECK(rd_data.size_bytes() == 7); // partial read
            CHECK(rd_buffer);
        }
        {
            auto&& [rd_data, rd_buffer] = stream.read_stable(18);
            CHECK(rd_data.size_bytes() == 17); // partial read
            CHECK(rd_buffer);
        }

        CHECK(stream.readable_byte_count() == 0);
    }

    SECTION("unread") {
        stream.write(make_data(14));
        stream.write(make_data(17));

        {
            auto&& [rd_data, rd_buffer] = stream.read_stable(7);
            CHECK(rd_data.size_bytes() == 7);
            CHECK(rd_buffer);
        }
        {
            auto&& [rd_data, rd_buffer] = stream.read_stable(9);
            CHECK(rd_data.size_bytes() == 7); // partial read
            CHECK(rd_buffer);
        }

        CHECK(stream.unread(14) == 14);
        // CHECK(stream.unread(19) == 0);
    }
}
