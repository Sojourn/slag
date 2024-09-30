#pragma once

#include <optional>
#include <vector>
#include <mutex>
#include "slag/core.h"
#include "slag/types.h"
#include "slag/thread.h"
#include "slag/topology.h"
#include "slag/bus.h"

namespace slag {

    class Thread;

    struct RuntimeConfig {
        std::optional<std::span<size_t>> gc_cpu_affinities = std::nullopt;
    };

    class Runtime : public Finalizer {
    public:
        explicit Runtime(const RuntimeConfig& config = RuntimeConfig{});
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
        RuntimeConfig                         config_;

        mutable std::recursive_mutex          mutex_;
        Domain                                domain_;
        std::shared_ptr<Fabric>               fabric_;
        std::vector<std::unique_ptr<Thread>>  threads_;
        std::vector<std::shared_ptr<Reactor>> reactors_;
    };

    template<typename RootTask, typename... Args>
    void Runtime::spawn_thread(const ThreadConfig& config, Args&&... args) {
        Thread& thread = [&]() -> Thread& {
            std::scoped_lock lock(mutex_);

            // The thread expects the corresponding reactor to be available upon construction.
            reactors_.push_back(std::make_shared<Reactor>());

            const ThreadIndex tidx = static_cast<ThreadIndex>(threads_.size());
            return *threads_.emplace_back(std::make_unique<Thread>(*this, tidx, config));
        }();

        thread.run<RootTask>(std::forward<Args>(args)...);
    }

}
