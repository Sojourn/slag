#include <cassert>

namespace slag {

    template<template<typename> class LayerImpl, typename StackImpl>
    inline auto Layer<LayerImpl, StackImpl>::above() -> Above* {
        return above_;
    }

    template<template<typename> class LayerImpl, typename StackImpl>
    inline auto Layer<LayerImpl, StackImpl>::above() const -> const Above* {
        return above_;
    }

    template<template<typename> class LayerImpl, typename StackImpl>
    inline auto Layer<LayerImpl, StackImpl>::below() -> Below* {
        return below_;
    }

    template<template<typename> class LayerImpl, typename StackImpl>
    inline auto Layer<LayerImpl, StackImpl>::below() const -> const Below* {
        return below_;
    }

    template<template<typename> class LayerImpl, typename StackImpl>
    inline void Layer<LayerImpl, StackImpl>::attach(StackImpl& stack) {
        above_ = stack.template get_layer_above<LayerImpl<StackImpl>>();
        below_ = stack.template get_layer_below<LayerImpl<StackImpl>>();
    }

    template<template<typename> class LayerImpl, typename StackImpl>
    inline void Layer<LayerImpl, StackImpl>::detach(StackImpl&) {
        above_ = nullptr;
        below_ = nullptr;
    }

}
