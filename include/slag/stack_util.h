#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include "slag/util.h"

namespace slag {

    template<int I, typename T>
    struct TupleElementAt;

    template<int I, typename T>
    struct TupleElementAt {
        using type = std::tuple_element_t<I, T>;
    };

    template<typename T>
    struct TupleElementAt<-1, T> {
        using type = void;
    };

    template<typename TargetLayer, typename... Layers>
    using layer_above_t = TupleElementAt<
        find_type_v<TargetLayer, Layers...> - 1,
        std::tuple<Layers...>
    >::type;

    template<typename TargetLayer, typename... Layers>
    struct LayerBelow {
        using ReverseTuple = tuple_reverse_t<std::tuple<Layers...>>;

        static constexpr int layer_count = static_cast<int>(sizeof...(Layers));
        static constexpr int forward_index = find_type_v<TargetLayer, Layers...>;
        static constexpr int reverse_index = (forward_index == (sizeof...(Layers) - 1)) ? -1 : (layer_count - forward_index - 2);

        using type = TupleElementAt<reverse_index, ReverseTuple>::type;
    };

    template<typename TargetLayer, typename... Layers>
    using layer_below_t = LayerBelow<TargetLayer, Layers...>::type;

    static_assert(std::is_same_v<char, layer_above_t<int, char, int>>);
    static_assert(std::is_same_v<void, layer_above_t<char, char, int>>);

    static_assert(std::is_same_v<void, layer_below_t<int, char, int>>);
    static_assert(std::is_same_v<int, layer_below_t<char, char, int>>);

}
