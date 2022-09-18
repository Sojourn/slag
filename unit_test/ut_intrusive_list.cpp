#include "catch.hpp"
#include "slag/slag.h"
#include <algorithm>
#include <initializer_list>

using namespace slag;

struct Item {
    int               value = 0;
    IntrusiveListNode node;
};
using ItemList = IntrusiveList<Item, &Item::node>;

bool operator==(const Item& l, int r) {
    return l.value == r;
}

bool equals(ItemList& items, std::initializer_list<int> expected_values) {
    if (std::distance(items.begin(), items.end()) != std::distance(expected_values.begin(), expected_values.end())) {
        return false;
    }

    auto item_pos = items.begin();
    auto item_end = items.end();
    auto expected_pos = expected_values.begin();
    auto expected_end = expected_values.end();

    while (item_pos != item_end) {
        assert(expected_pos != expected_end);
        if (item_pos->value != *expected_pos) {
            return false;
        }

        ++item_pos;
        ++expected_pos;
    }

    return true;
}


TEST_CASE("IntrusiveList", "basic") {
    static constexpr int ITEM_COUNT = 3;

    ItemList list;
    Item items[ITEM_COUNT];
    for (int i = 0; i < ITEM_COUNT; ++i) {
        items[i].value = i;
    }

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

    SECTION("iterator increment/decrement") {
        list.push_back(items[0]);
        list.push_back(items[1]);
        list.push_back(items[2]);

        auto it = list.begin();
        CHECK(*it == 0);
        CHECK(*(it++) == 0);
        CHECK(*(++it) == 2);
        CHECK(*it == 2);

        it = list.end();
        CHECK(*(--it) == 2);
        CHECK(*(it--) == 2);
        CHECK(*(it) == 1);

        CHECK(equals(list, {0, 1, 2}));
    }

    SECTION("erase while iterating") {
        list.push_back(items[0]);
        list.push_back(items[1]);
        list.push_back(items[2]);

        auto it = list.begin();

        it = list.erase(it);
        CHECK(equals(list, {1, 2}));

        it = list.erase(it);
        CHECK(equals(list, {2}));

        it = list.erase(it);
        CHECK(equals(list, {}));

        // erase(end()) should nop
        it = list.erase(it);
        CHECK(equals(list, {}));
    }

    SECTION("moving a list") {
        list.push_back(items[0]);
        list.push_back(items[1]);
        list.push_back(items[2]);

        // move construct
        ItemList new_list{std::move(list)};
        CHECK(equals(list, {}));
        CHECK(equals(new_list, {0, 1, 2}));

        // move assign
        list = std::move(new_list);
        CHECK(equals(list, {0, 1, 2}));
        CHECK(equals(new_list, {}));
    }

    SECTION("moving an element") {
        list.push_back(items[0]);
        list.push_back(items[1]);
        list.push_back(items[2]);

        // move construct
        Item new_item0 = std::move(items[0]);
        CHECK(new_item0.node.is_linked());
        CHECK(!items[0].node.is_linked());
        CHECK(equals(list, {0, 1, 2}));

        // move assign
        items[0] = std::move(new_item0);
        CHECK(!new_item0.node.is_linked());
        CHECK(items[0].node.is_linked());
        CHECK(equals(list, {0, 1, 2}));
    }
}
