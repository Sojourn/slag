#include "catch.hpp"
#include "slag/message.h"

using namespace slag;

TEST_CASE("Message") {
    SECTION("Reading and Writing") {
        Message message;

        {
            MessageWriter writer{message};

            writer.write_slot(5);
            writer.write_text("hello");
            writer.write_flag(false);
            writer.write_slot(6);
            writer.write_text("world!");
            writer.write_flag(true);
            writer.write_flag(true);
        }
        {
            MessageReader reader{message};

            CHECK(reader.read_slot() == 5);
            CHECK(reader.read_text(5) == "hello");
            CHECK(reader.read_flag() == false);
            CHECK(reader.read_slot() == 6);
            CHECK(reader.read_text(6) == "world!");
            CHECK(reader.read_flag() == true);
            CHECK(reader.read_flag() == true);
        }

        CHECK(message.appendage().size_bytes() == 11); // "helloworld!";
        CHECK(message.slots().size() == 3);
        CHECK(message.slots()[0] == 5);
        CHECK(message.slots()[2] == 6);
    }
}
