#include <cassert>

namespace slag {

    template<typename Visitor, typename T>
    constexpr inline void visit(Visitor&& visitor, const T& value) {
        visitor.enter(value);
        visitor.leave(value);
    }

    template<typename Visitor, typename T>
    constexpr inline void visit(Visitor&& visitor, const std::optional<T>& value) {
        visitor.enter(value);
        if (value) {
            visit(visitor, *value);
        }
        visitor.leave(value);
    }

    template<typename Visitor, typename T1, typename T2>
    constexpr inline void visit(Visitor&& visitor, const std::pair<T1, T2>& value) {
        visitor.enter(value);
        {
            visit(visitor, value.first);
            visit(visitor, value.second);
        }
        visitor.leave(value);
    }

    template<typename Visitor, typename... Types>
    constexpr inline void visit(Visitor&& visitor, const std::tuple<Types...>& value) {
        visitor.enter(value);
        {
            using IndexSequence = std::make_index_sequence<sizeof...(Types)>;

            [&]<size_t... I>(std::index_sequence<I...>) {
                (visit(visitor, std::get<I>(value)), ...);
            }(IndexSequence{});
        }
        visitor.leave(value);
    }

    template<typename Visitor, typename... Types>
    constexpr inline void visit(Visitor&& visitor, const std::variant<Types...>& value) {
        visitor.enter(value);
        std::visit([&](auto&& element) {
            visit(visitor, element);
        }, value);
        visitor.leave(value);
    }

    template<typename Visitor, typename T>
    constexpr inline void visit(Visitor&& visitor, const std::vector<T>& value) {
        visitor.enter(value);
        for (auto&& element: value) {
            visit(visitor, element);
        }
        visitor.leave(value);
    }

    template<typename Visitor, typename Key, typename T>
    constexpr inline void visit(Visitor&& visitor, const std::unordered_map<Key, T>& value) {
        visitor.enter(value);
        for (auto&& element: value) {
            visit(visitor, element); // visit the key/value pair
        }
        visitor.leave(value);
    }

}