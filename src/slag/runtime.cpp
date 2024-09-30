#include "runtime.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Runtime::Runtime(const RuntimeConfig& config)
        : config_(config)
        , domain_(config_.gc_cpu_affinities)
        , fabric_(std::make_shared<Fabric>())
    {
    }

    Runtime::~Runtime() {
        size_t thread_count;
        {
            std::scoped_lock lock(mutex_);
            thread_count = threads_.size();
        }
        if (thread_count == 0) {
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
        std::scoped_lock lock(mutex_);

        return fabric_;
    }

    std::shared_ptr<Reactor> Runtime::reactor(ThreadIndex thread_index) {
        std::scoped_lock lock(mutex_);

        return reactors_.at(thread_index);
    }

    void Runtime::finalize(ObjectGroup, std::span<Object*>) noexcept {
        abort();
    }

}
