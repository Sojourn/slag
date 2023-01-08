#include <algorithm>
#include <initializer_list>
#include <cassert>
#include "slag/slag.h"

using namespace slag;

struct Item {
    int value = 0;
    IntrusiveListNode node;
};
using ItemList = IntrusiveList<Item, &Item::node>;

std::string to_string(ItemList& list) {
    std::string result = "[";

    size_t item_count = 0;
    for (auto&& item: list) {
        if (item_count++) {
            result += ", ";
        }

        result += fmt::format("{}", item.value);
    }

    result += "]";
    return result;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    srand(time(NULL));

    int item_sum = 0;

    ItemList list1;
    ItemList list2;

    std::vector<Item> items;
    items.resize(5);
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

    for (size_t i = 0; i < 100000000; ++i) {
        // if ((i % 100000) == 0) {
        //     std::cout << i << std::endl;
        //     std::cout << "  list1: " << to_string(list1) << std::endl;
        //     std::cout << "  list2: " << to_string(list2) << std::endl;
        // }

        // audit the lists
        {
            for (auto&& item: items) {
                assert(item.node.is_linked());
            }

            int expected_item_sum = item_sum;
            for (auto&& item: list1) {
                expected_item_sum -= item.value;
            }
            for (auto&& item: list2) {
                expected_item_sum -= item.value;
            }
            assert(expected_item_sum == 0);
        }

        switch (rand() % 14) {
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
                assert(item.node.is_linked());
                item.node.unlink();
                list1.push_back(item);
                break;
            }
            case 5: {
                Item& item = items[rand() % items.size()];
                assert(item.node.is_linked());
                item.node.unlink();
                list2.push_back(item);
                break;
            }
            case 6: {
                Item& item = items[rand() % items.size()];
                assert(item.node.is_linked());
                item.node.unlink();
                list1.push_front(item);
                break;
            }
            case 7: {
                Item& item = items[rand() % items.size()];
                assert(item.node.is_linked());
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
            case 10: {
                Item& item1 = items[rand() % items.size()];
                int value = item1.value;
                item1.~Item();

                Item& item2 = *(new(&item1) Item);
                item2.value = value;
                list1.push_back(item2);
                break;
            }
            case 11: {
                Item& item1 = items[rand() % items.size()];
                int value = item1.value;
                item1.~Item();

                Item& item2 = *(new(&item1) Item);
                item2.value = value;
                list2.push_back(item2);
                break;
            }
            case 12: {
                // std::cout << "[reverse-list1]" << std::endl;
                // std::cout << "  " << to_string(list1) << std::endl;

                std::vector<Item*> temp_items;
                for (auto it = list1.begin(); it != list1.end(); ) {
                    temp_items.push_back(&*it);
                    it = list1.erase(it);
                }

                assert(list1.is_empty());
                for (Item* item: temp_items) {
                    list1.push_front(*item);
                }

                // std::cout << "  " << to_string(list1) << std::endl;
                break;
            }
            case 13: {
                // std::cout << "[reverse-list2]" << std::endl;
                // std::cout << "  " << to_string(list2) << std::endl;

                std::vector<Item*> temp_items;
                for (auto it = list2.begin(); it != list2.end(); ) {
                    temp_items.push_back(&*it);
                    it = list2.erase(it);
                }

                assert(list2.is_empty());
                for (Item* item: temp_items) {
                    list2.push_front(*item);
                }

                // std::cout << "  " << to_string(list2) << std::endl;
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

    assert(item_sum == 0);

    return 0;
}
