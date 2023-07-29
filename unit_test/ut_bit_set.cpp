#include "catch.hpp"
#include "slag/slag.h"
#include <cstring>

using namespace slag;

TEST_CASE("BitSet") {
    SECTION("Setting, Reseting and Testing") {
        BitSet bits{1 << 16};

        // set a handful of bits at random
        bits.set(1);
        bits.set(10337);
        bits.set(94);

        // test the bits we've set and their neighbors
        {
            CHECK(!bits.test(0));
            CHECK(bits.test(1));
            CHECK(!bits.test(2));

            CHECK(!bits.test(10336));
            CHECK(bits.test(10337));
            CHECK(!bits.test(10338));

            CHECK(!bits.test(93));
            CHECK(bits.test(94));
            CHECK(!bits.test(95));
        }

        // reset and unset bit
        CHECK(!bits.test(13));
        bits.reset(13);
        CHECK(!bits.test(13));

        // reset a set bit
        CHECK(bits.test(94));
        bits.reset(94);
        CHECK(!bits.test(94));
    }

    SECTION("Scanning") {
        BitSet bits{1 << 16};

        // set a handful of bits at random
        bits.set(14);
        bits.set(1022);
        bits.set(233);
        bits.set(13038);

        BitSetScanner scanner{bits};

        CHECK(scanner.next() == 14);
        CHECK(scanner.next() == 233);
        CHECK(scanner.next() == 1022);
        CHECK(scanner.next() == 13038);
        CHECK(!scanner.next());
    }
}
