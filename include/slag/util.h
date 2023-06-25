#pragma once

#include <tuple>
#include <utility>
#include <concepts>
#include <type_traits>
#include <cstddef>

namespace slag {

    template<typename T>
    struct TypeWrapper {
        using type = T;
    };

    template<size_t I>
    struct IndexWrapper {
        static constexpr size_t value = I;
    };

    template<int I, typename TargetType, typename... Types>
    struct FindType;

    template<int I, typename TargetType>
    struct FindType<I, TargetType> {
        static constexpr int value = -1; // base case
    };

    template<int I, typename TargetType, typename Type, typename... Types>
    struct FindType<I, TargetType, Type, Types...> {
        static constexpr int value = (
            std::is_same_v<TargetType, Type> ? I : FindType<I+1, TargetType, Types...>::value
        );
    };

    template<typename TargetType, typename... Types>
    constexpr int find_type_v = FindType<0, TargetType, Types...>::value;

    template<typename T, typename IndexSequence>
    struct tuple_reverse_impl;

    template<typename T, size_t... I>
    struct tuple_reverse_impl<T, std::index_sequence<I...>> {
        using type = std::tuple<typename std::tuple_element<sizeof...(I) - 1 - I, T>::type...>;
    };

    // handle empty tuples
    template<typename T>
    struct tuple_reverse_impl<T, std::index_sequence<>> {
        using type = T;
    };

    template<typename T>
    using tuple_reverse_t = tuple_reverse_impl<T, std::make_index_sequence<std::tuple_size<T>::value>>::type;

    // raise a system error for the currently set errno
    void raise_system_error(const char* message);

}
