#pragma once

#include <optional>
#include <utility>
#include <tuple>
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstddef>
#include <cstdint>

namespace slag {

    template<typename Visitor, typename T>
    constexpr void visit(Visitor&& visitor, T& value);

    template<typename Visitor, typename T>
    constexpr void visit(Visitor&& visitor, const T& value);

    template<typename Visitor, typename T>
    constexpr void visit(Visitor&& visitor, std::optional<T>& value);

    template<typename Visitor, typename T>
    constexpr void visit(Visitor&& visitor, const std::optional<T>& value);

    template<typename Visitor, typename T1, typename T2>
    constexpr void visit(Visitor&& visitor, std::pair<T1, T2>& value);

    template<typename Visitor, typename T1, typename T2>
    constexpr void visit(Visitor&& visitor, const std::pair<T1, T2>& value);

    template<typename Visitor, typename... Types>
    constexpr void visit(Visitor&& visitor, std::tuple<Types...>& value);

    template<typename Visitor, typename... Types>
    constexpr void visit(Visitor&& visitor, const std::tuple<Types...>& value);

    template<typename Visitor, typename... Types>
    constexpr void visit(Visitor&& visitor, std::variant<Types...>& value);

    template<typename Visitor, typename... Types>
    constexpr void visit(Visitor&& visitor, const std::variant<Types...>& value);

    template<typename Visitor, typename T>
    constexpr void visit(Visitor&& visitor, std::vector<T>& value);

    template<typename Visitor, typename T>
    constexpr void visit(Visitor&& visitor, const std::vector<T>& value);

    template<typename Visitor, typename Key, typename T>
    constexpr void visit(Visitor&& visitor, std::unordered_map<Key, T>& value);

    template<typename Visitor, typename Key, typename T>
    constexpr void visit(Visitor&& visitor, const std::unordered_map<Key, T>& value);

}

#include "visit.hpp"
