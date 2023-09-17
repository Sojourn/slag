#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include "slag/util.h"
#include "slag/stack_util.h"
#include "slag/layer.h"

namespace slag {

    template<template<typename> class... Layers>
    class Stack {
        Stack(Stack&&) = delete;
        Stack(const Stack&) = delete;
        Stack& operator=(Stack&&) = delete;
        Stack& operator=(const Stack&) = delete;

    public:
        Stack();
        ~Stack();

    public:
        using LayersTuple = std::tuple<
            Layers<Stack<Layers...>>...
        >;

        template<size_t index>
        [[nodiscard]] std::tuple_element_t<index, LayersTuple>& get_layer_at();

        template<size_t index>
        [[nodiscard]] const std::tuple_element_t<index, LayersTuple>& get_layer_at() const;

        [[nodiscard]] std::tuple_element_t<0, LayersTuple>& get_top_layer();
        [[nodiscard]] const std::tuple_element_t<0, LayersTuple>& get_top_layer() const;
        [[nodiscard]] std::tuple_element_t<sizeof...(Layers) - 1, LayersTuple>& get_bottom_layer();
        [[nodiscard]] const std::tuple_element_t<sizeof...(Layers) - 1, LayersTuple>& get_bottom_layer() const;

        template<typename Functor>
        void for_each_layer(Functor&& functor);

        template<typename Functor>
        void for_each_layer_reverse(Functor&& functor);

    private:
        template<template<typename> class LayerImpl, typename StackImpl>
        friend class Layer;

        template<typename Layer>
        using LayerAboveType = layer_above_t<
            Layer,
            Layers<Stack<Layers...>>...
        >;

        template<typename Layer>
        using LayerBelowType = layer_below_t<
            Layer,
            Layers<Stack<Layers...>>...
        >;

        template<typename LayerImpl>
        [[nodiscard]] LayerAboveType<LayerImpl>* get_layer_above();

        template<typename LayerImpl>
        [[nodiscard]] LayerBelowType<LayerImpl>* get_layer_below();

    private:
        LayersTuple layers_;
    };

}

#include "stack.hpp"
