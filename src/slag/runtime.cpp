#include "runtime.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Runtime::Runtime(const RuntimeConfig& config)
        : config_(config)
        , domain_(config_.gc_cpu_affinities)
        , region_(domain_, *this)
        , fabric_(std::make_shared<Fabric>(config_.thread_topology))
    {
    }

    Runtime::~Runtime() {
        fabric_.reset();
        region_.stop();
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

    void Runtime::finalize(ObjectGroup group, std::span<Object*> objects) noexcept {
        switch (static_cast<ResourceType>(group)) {
            case ResourceType::MANAGED: {
                for (Object* object : objects) {
                    delete static_cast<Managed*>(object);
                }
                break;
            }
            default: {
                abort();
            }
        }
    }

}
