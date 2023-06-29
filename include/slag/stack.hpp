#include <cassert>

namespace slag {

    template<template<typename> class... Layers>
    inline Stack<Layers...>::Stack() {
        for_each_layer([this](auto&& layer) {
            layer.attach(*this);
        });

        for_each_layer([this](auto&& layer) {
            layer.start();
        });
    }

    template<template<typename> class... Layers>
    inline Stack<Layers...>::~Stack() {
        for_each_layer([this](auto&& layer) {
            layer.stop();
        });

        for_each_layer([this](auto&& layer) {
            layer.detach(*this);
        });
    }

    template<template<typename> class... Layers>
    template<size_t index>
    inline auto Stack<Layers...>::get_layer_at() -> std::tuple_element_t<index, LayersTuple>& {
        return std::get<index>(layers_);
    }

    template<template<typename> class... Layers>
    template<size_t index>
    inline auto Stack<Layers...>::get_layer_at() const -> const std::tuple_element_t<index, LayersTuple>& {
        return std::get<index>(layers_);
    }

    template<template<typename> class... Layers>
    inline auto Stack<Layers...>::get_top_layer() -> std::tuple_element_t<0, LayersTuple>& {
        return std::get<0>(layers_);
    }

    template<template<typename> class... Layers>
    inline auto Stack<Layers...>::get_top_layer() const -> const std::tuple_element_t<0, LayersTuple>& {
        return std::get<0>(layers_);
    }

    template<template<typename> class... Layers>
    inline auto Stack<Layers...>::get_bottom_layer() -> std::tuple_element_t<sizeof...(Layers) - 1, LayersTuple>& {
        return std::get<sizeof...(Layers) - 1>(layers_);
    }

    template<template<typename> class... Layers>
    inline auto Stack<Layers...>::get_bottom_layer() const -> const std::tuple_element_t<sizeof...(Layers) - 1, LayersTuple>& {
        return std::get<sizeof...(Layers) - 1>(layers_);
    }

    template<template<typename> class... Layers>
    template<typename Functor>
    inline void Stack<Layers...>::for_each_layer(Functor&& functor) {
        auto visit = [&]<size_t... I>(std::index_sequence<I...>) {
            (functor(get_layer_at<I>()), ...);
        };

        visit(std::make_index_sequence<sizeof...(Layers)>{});
    }

    template<template<typename> class... Layers>
    template<typename LayerImpl>
    inline auto Stack<Layers...>::get_layer_above() -> LayerAboveType<LayerImpl>* {
        using T = LayerAboveType<LayerImpl>;
        static_assert(!std::is_same_v<T, LayerImpl>);

        if constexpr (std::is_same_v<T, void>) {
            return static_cast<void*>(nullptr); // for return type deduction
        }
        else {
            return &std::get<T>(layers_);
        }
    }

    template<template<typename> class... Layers>
    template<typename LayerImpl>
    inline auto Stack<Layers...>::get_layer_below() -> LayerBelowType<LayerImpl>* {
        using T = LayerBelowType<LayerImpl>;
        static_assert(!std::is_same_v<T, LayerImpl>);

        if constexpr (std::is_same_v<T, void>) {
            return static_cast<void*>(nullptr); // for return type deduction
        }
        else {
            return &std::get<T>(layers_);
        }
    }

}
