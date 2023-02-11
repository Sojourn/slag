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

TEST_CASE("Fuzz IntrusiveList") {
    int item_sum = 0;

    ItemList list1;
    ItemList list2;

    std::vector<Item> items;
    items.resize(1000);
    for (size_t i = 0 ; i < items.size(); ++i) {
        Item& item = items[i];

        item.value = static_cast<int>(i);
        item_sum += item.value;

        if ((i % 2) == 0) {
            list1.push_back(item);
        }
        else {
            list2.push_back(item);
        }
    }

    for (size_t i = 0; i < 1000; ++i) {
        CHECK(static_cast<size_t>(std::distance(list1.begin(), list1.end()) + std::distance(list2.begin(), list2.end())) == items.size());
        for (auto&& item: items) {
            CHECK(item.node.is_linked());
        }

        switch (rand() % 5) {
            case 0: {
                if (!list1.is_empty()) {
                    list2.push_back(list1.pop_front());
                }
                break;
            }
            case 1: {
                if (!list2.is_empty()) {
                    list1.push_back(list2.pop_front());
                }
                break;
            }
            case 2: {
                if (!list1.is_empty()) {
                    list2.push_front(list1.pop_back());
                }
                break;
            }
            case 3: {
                if (!list2.is_empty()) {
                    list1.push_front(list2.pop_back());
                }
                break;
            }
            case 4: {
                Item& item = items[rand() % items.size()];
                CHECK(item.node.is_linked());
                item.node.unlink();
                list1.push_back(item);
                break;
            }
            case 5: {
                Item& item = items[rand() % items.size()];
                CHECK(item.node.is_linked());
                item.node.unlink();
                list2.push_back(item);
                break;
            }
            case 6: {
                Item& item = items[rand() % items.size()];
                CHECK(item.node.is_linked());
                item.node.unlink();
                list1.push_front(item);
                break;
            }
            case 7: {
                Item& item = items[rand() % items.size()];
                CHECK(item.node.is_linked());
                item.node.unlink();
                list2.push_front(item);
                break;
            }
            case 8: {
                std::swap(list1, list2);
                break;
            }
            case 9: {
                Item& item1 = items[rand() % items.size()];
                Item& item2 = items[rand() % items.size()];
                std::swap(item1, item2);
                break;
            }

            default: {
                break;
            }
        }
    }

    for (const Item& item: list1) {
        item_sum -= item.value;
    }
    for (const Item& item: list2) {
        item_sum -= item.value;
    }

    CHECK(item_sum == 0);
}

TEST_CASE("Basic IntrusiveList") {
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

        // move construct
        ItemList new_list{std::move(list)};
        CHECK(equals(list, {}));
        CHECK(equals(new_list, {0}));

        // move assign
        list = std::move(new_list);
        list.push_back(items[1]);
        new_list.push_back(items[2]);
        CHECK(equals(list, {0, 1}));
        CHECK(equals(new_list, {2}));
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

    SECTION("auto-unlinking an element") {
        {
            list.push_back(items[0]);
            {
                Item temp_item = {
                    .value = 14,
                    .node  = {},
                };
                list.push_back(temp_item);

                CHECK(list.front().value == 0);
                CHECK(list.back().value == 14);
            }

            CHECK(list.size() == 1);
            CHECK(list.front().value == 0);
            CHECK(list.back().value == 0);

            list.clear();
        }

        // temp (front)
        {
            struct ItemWrapper {
                Item item = {
                    .value = 13,
                    .node  = {},
                };

                Item& item_ref() {
                    return item;
                }
            };

            list.push_back(ItemWrapper{}.item_ref());
            list.push_back(items[2]);

            CHECK(list.size() == 1);
            CHECK(list.front().value == 2);
            CHECK(list.back().value == 2);

            list.clear();
        }

        // temp (middle)
        {
            struct ItemWrapper {
                Item item = {
                    .value = 13,
                    .node  = {},
                };

                Item& item_ref() {
                    return item;
                }
            };

            list.push_front(items[1]);
            list.push_back(ItemWrapper{}.item_ref());
            list.push_back(items[2]);

            CHECK(list.size() == 2);
            CHECK(list.front().value == 1);
            CHECK(list.back().value == 2);

            list.clear();
        }

        // temp (back)
        {
            struct ItemWrapper {
                Item item = {
                    .value = 13,
                    .node  = {},
                };

                Item& item_ref() {
                    return item;
                }
            };

            list.push_front(items[1]);
            list.push_back(ItemWrapper{}.item_ref());

            CHECK(list.size() == 1);
            CHECK(list.front().value == 1);
            CHECK(list.back().value == 1);

            list.clear();
        }
    }
}
