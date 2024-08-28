#pragma once

#include <optional>
#include <vector>
#include <latch>
#include "slag/core.h"
#include "slag/types.h"
#include "slag/thread.h"
#include "slag/topology.h"

namespace slag {

    class Thread;

    class Runtime : public Finalizer {
    public:
        explicit Runtime(const ThreadGraph& thread_graph);
        ~Runtime();

        Runtime(Runtime&&) = delete;
        Runtime(const Runtime&) = delete;
        Runtime& operator=(Runtime&&) = delete;
        Runtime& operator=(const Runtime&) = delete;

        Domain& domain();

        template<typename RootTask, typename... Args>
        void spawn_thread(const ThreadConfig& config, Args&&... args);

    private:
        void finalize(ObjectGroup group, std::span<Object*> objects) noexcept override;

    private:
        Domain                               domain_;
        Region                               region_;

        std::vector<std::unique_ptr<Thread>> threads_;
        ThreadGraph                          thread_graph_;
    };

    template<typename RootTask, typename... Args>
    void Runtime::spawn_thread(const ThreadConfig& config, Args&&... args) {
        auto&& thread = threads_.emplace_back(std::make_unique<Thread>(*this, config));

        thread->run<RootTask>(std::forward<Args>(args)...);
    }

}
