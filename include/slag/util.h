#pragma once

#include <vector>
#include <algorithm>
#include <cstddef>

namespace slag {
    template<typename T, typename Functor>
    inline void stable_for_each(std::vector<T>& collection, Functor&& functor) {
        std::size_t i = 0;
        while (true) {
            std::size_t size = collection.size();
            if (i == size) {
                break;
            }

            functor(collection[size - i - 1]);
        }
    }

    template<typename T>
    inline bool swap_and_pop(std::vector<T>& collection, const T& item) {
        auto it = std::find(collection.begin(), collection.end(), item);
        if (it == collection.end()) {
            return false;
        }

        std::swap(
            collection[it - collection.begin()],
            collection[collection.size() - 1]
        );

        collection.pop_back();
        return true;
    }
}
