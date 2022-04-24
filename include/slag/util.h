#pragma once

#include <vector>
#include <cstddef>

namespace slag {
    template<typename T, typename Functor>
    inline void for_each_safe(std::vector<T>& collection, Functor&& functor) {
        std::size_t i = 0;
        while (true) {
            std::size_t size = collection.size();
            if (i == size) {
                break;
            }

            functor(collection[size - i - 1]);
        }
    }
}
