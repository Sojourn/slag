#pragma once

#include <optional>
#include <vector>
#include <latch>
#include "slag/core.h"
#include "slag/types.h"
#include "slag/thread.h"
#include "slag/topology.h"
#include "slag/bus.h"

namespace slag {

    class Thread;

    struct RuntimeConfig {
        ThreadGraph                      thread_topology;
        std::optional<std::span<size_t>> gc_cpu_affinities = std::nullopt;
    };

    class Runtime : public Finalizer {
    public:
        explicit Runtime(const RuntimeConfig& config);
        ~Runtime();

        Runtime(Runtime&&) = delete;
        Runtime(const Runtime&) = delete;
        Runtime& operator=(Runtime&&) = delete;
        Runtime& operator=(const Runtime&) = delete;

        const RuntimeConfig& config() const;

        Domain& domain();

        std::shared_ptr<Fabric> fabric();

        template<typename RootTask, typename... Args>
        void spawn_thread(const ThreadConfig& config, Args&&... args);

    private:
        void finalize(ObjectGroup group, std::span<Object*> objects) noexcept override;

    private:
        RuntimeConfig                        config_;
        Domain                               domain_;
        Region                               region_;
        std::shared_ptr<Fabric>              fabric_;
        std::vector<std::unique_ptr<Thread>> threads_;
    };

    template<typename RootTask, typename... Args>
    void Runtime::spawn_thread(const ThreadConfig& config, Args&&... args) {
        auto&& thread = threads_.emplace_back(std::make_unique<Thread>(*this, config));

        thread->run<RootTask>(std::forward<Args>(args)...);
    }

}
