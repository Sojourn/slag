#include "runtime.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Runtime::Runtime(const RuntimeConfig& config)
        : config_(config)
        , domain_(config_.gc_cpu_affinities)
        , region_(domain_, *this)
    {
    }

    Runtime::~Runtime() {
        region_.stop();
    }

    const RuntimeConfig& Runtime::config() const {
        return config_;
    }

    Domain& Runtime::domain() {
        return domain_;
    }

    void Runtime::finalize(ObjectGroup, std::span<Object*>) noexcept {
        abort(); // Override this if needed.
    }

}
