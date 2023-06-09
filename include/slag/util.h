#pragma once

#include <concepts>
#include <type_traits>

namespace slag {

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

    // raise a system error for the currently set errno
    void raise_system_error(const char* message);

}
