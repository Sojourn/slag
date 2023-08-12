#include "catch.hpp"
#include "slag/slag.h"
#include <cstring>

using namespace slag;

struct Element {
    int                value;
    IntrusiveQueueNode node1;
    IntrusiveQueueNode node2;

    Element(int value): value(value) {}
};

using Queue1 = IntrusiveQueue<Element, &Element::node1>;
using Queue2 = IntrusiveQueue<Element, &Element::node2>;

TEST_CASE("IntrusiveQueue") {
    SECTION("Pushing and Popping") {
        Queue1 queue;

        Element item1(1);
        Element item2(2);
        Element item3(3);

        queue.push_back(item1);
        CHECK(queue.size() == 1);

        queue.push_back(item2);
        CHECK(queue.size() == 2);

        queue.push_back(item3);
        CHECK(queue.size() == 3);

        CHECK(queue.pop_front()->value == 1);
        CHECK(queue.pop_front()->value == 2);
        CHECK(queue.pop_front()->value == 3);
        CHECK(queue.size() == 0);
    }
}
