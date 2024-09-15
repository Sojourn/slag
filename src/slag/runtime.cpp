#include "runtime.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Runtime::Runtime(const RuntimeConfig& config)
        : config_(config)
        , domain_(config_.gc_cpu_affinities)
        , fabric_(std::make_shared<Fabric>(config_.thread_topology))
    {
    }

    Runtime::~Runtime() {
        if (threads_.empty()) {
            // Unblock the domain so we can shutdown.
            Region dummy_region(domain_, *this);
            dummy_region.stop();
        }

        fabric_.reset();
    }

    const RuntimeConfig& Runtime::config() const {
        return config_;
    }

    Domain& Runtime::domain() {
        return domain_;
    }

    std::shared_ptr<Fabric> Runtime::fabric() {
        return fabric_;
    }

    std::shared_ptr<Reactor> Runtime::reactor(ThreadIndex thread_index) {
        if (std::shared_ptr<Reactor>& reactor = reactors_.at(thread_index)) {
            return reactor;
        }

        throw std::runtime_error("Inactive reactor");
    }

    void Runtime::finalize(ObjectGroup, std::span<Object*>) noexcept {
        abort();
    }

}
