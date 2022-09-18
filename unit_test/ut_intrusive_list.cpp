#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

struct Item {
    int               value = 0;
    IntrusiveListNode node;
};

TEST_CASE("IntrusiveList", "basic") {
    static constexpr int ITEM_COUNT = 3;

    Item items[ITEM_COUNT];
    for (int i = 0; i < ITEM_COUNT; ++i) {
        items[i].value = i;
    }

    IntrusiveList<Item, &Item::node> list;

    SECTION("pushing and popping") {
        list.push_back(items[0]);
        list.push_back(items[1]);
        list.push_back(items[2]);
        CHECK(list.front().value == 0);
        CHECK(list.back().value == 2);

        CHECK(list.pop_back().value == 2);
        CHECK(list.front().value == 0);
        CHECK(list.back().value == 1);

        CHECK(list.pop_front().value == 0);
        CHECK(list.front().value == 1);
        CHECK(list.back().value == 1);

        CHECK(list.pop_front().value == 1);
        CHECK(list.is_empty());
    }
}
