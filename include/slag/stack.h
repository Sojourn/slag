#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include "slag/util.h"
#include "slag/stack_util.h"

namespace slag {

    template<template<typename> class... Layers>
    class Stack {
    public:
        Stack(Stack&&) = delete;
        Stack(const Stack&) = delete;
        Stack& operator=(Stack&&) = delete;
        Stack& operator=(const Stack&) = delete;

        using LayersTuple = std::tuple<
            Layers<Stack<Layers...>>...
        >;

    public:
        template<size_t index>
        [[nodiscard]] std::tuple_element_t<index, LayersTuple>& get_layer_at() {
            return std::get<index>(layers_);
        }

        template<size_t index>
        [[nodiscard]] const std::tuple_element_t<index, LayersTuple>& get_layer_at() const {
            return std::get<index>(layers_);
        }

        template<typename Functor>
        void for_each_layer(Functor&& functor) {
            auto visit = [&]<size_t... I>(std::index_sequence<I...>) {
                (functor(get_layer_at<I>()), ...);
            };

            visit(std::make_index_sequence<sizeof...(Layers)>{});
        }

    public:
        Stack() {
            for_each_layer([this](auto&& layer) {
                layer.attach(*this);
            });

            for_each_layer([this](auto&& layer) {
                layer.start();
            });
        }

        ~Stack() {
            for_each_layer([this](auto&& layer) {
                layer.stop();
            });

            for_each_layer([this](auto&& layer) {
                layer.detach(*this);
            });
        }

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
        [[nodiscard]] auto* get_layer_above() {
            using T = LayerAboveType<LayerImpl>;
            static_assert(!std::is_same_v<T, LayerImpl>);

            if constexpr (std::is_same_v<T, void>) {
                return static_cast<void*>(nullptr); // for return type deduction
            }
            else {
                return &std::get<T>(layers_);
            }
        }

        template<typename LayerImpl>
        [[nodiscard]] auto* get_layer_below() {
            using T = LayerBelowType<LayerImpl>;
            static_assert(!std::is_same_v<T, LayerImpl>);

            if constexpr (std::is_same_v<T, void>) {
                return static_cast<void*>(nullptr); // for return type deduction
            }
            else {
                return &std::get<T>(layers_);
            }
        }

    private:
        LayersTuple layers_;
    };

    template<template<typename> class LayerImpl, typename StackImpl>
    class Layer {
    public:
        using Above = typename StackImpl::LayerAboveType<LayerImpl<StackImpl>>;
        using Below = typename StackImpl::LayerBelowType<LayerImpl<StackImpl>>;

        [[nodiscard]] Above* above() {
            return above_;
        }

        [[nodiscard]] const Above* above() const {
            return above_;
        }

        [[nodiscard]] Below* below() {
            return below_;
        }

        [[nodiscard]] const Below* below() const {
            return below_;
        }

        void start() {
            // shadow this if you want the lifetime event hook
        }

        void stop() {
            // shadow this if you want the lifetime event hook
        }

    private:
        template<template<typename> class... Layers>
        friend class Stack;

        void attach(StackImpl& stack) {
            above_ = stack.template get_layer_above<LayerImpl<StackImpl>>();
            below_ = stack.template get_layer_below<LayerImpl<StackImpl>>();
        }

        void detach(StackImpl&) {
            above_ = nullptr;
            below_ = nullptr;
        }

        Above* above_ = nullptr;
        Below* below_ = nullptr;
    };

}
