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
        std::shared_ptr<Reactor> reactor(ThreadIndex thread_index);

        template<typename RootTask, typename... Args>
        void spawn_thread(const ThreadConfig& config, Args&&... args);

    private:
        void finalize(ObjectGroup group, std::span<Object*> objects) noexcept override;

    private:
        using ThreadArray = std::array<std::unique_ptr<Thread>, MAX_THREAD_COUNT>;
        using ReactorArray = std::array<std::shared_ptr<Reactor>, MAX_THREAD_COUNT>;

        RuntimeConfig           config_;
        Domain                  domain_;
        std::shared_ptr<Fabric> fabric_;
        ReactorArray            reactors_;
        ThreadArray             threads_;
    };

    template<typename RootTask, typename... Args>
    void Runtime::spawn_thread(const ThreadConfig& config, Args&&... args) {
        if (threads_[config.index]) {
            throw std::runtime_error("Thread has already been started");
        }

        threads_[config.index] = std::make_unique<Thread>(*this, config);
        threads_[config.index]->run<RootTask>(std::forward<Args>(args)...);
    }

}
