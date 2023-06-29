#pragma once

namespace slag {

    template<template<typename> class LayerImpl, typename StackImpl>
    class Layer {
    protected:
        using Above = typename StackImpl::LayerAboveType<LayerImpl<StackImpl>>;
        using Below = typename StackImpl::LayerBelowType<LayerImpl<StackImpl>>;

        [[nodiscard]] Above* above();
        [[nodiscard]] const Above* above() const;
        [[nodiscard]] Below* below();
        [[nodiscard]] const Below* below() const;

        void start();
        void stop();

    private:
        template<template<typename> class... Layers>
        friend class Stack;

        void attach(StackImpl& stack);
        void detach(StackImpl&);

    private:
        Above* above_    = nullptr;
        Below* below_    = nullptr;
    };

}

#include "layer.hpp"
