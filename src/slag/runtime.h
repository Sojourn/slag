#pragma once

#include <optional>
#include <vector>
#include <latch>
#include "slag/core.h"
#include "slag/types.h"
#include "thread.h"

namespace slag {

    class Thread;

    class Runtime : public Finalizer {
    public:
        Runtime(int argc, char** argv);
        ~Runtime();

        Runtime(Runtime&&) = delete;
        Runtime(const Runtime&) = delete;
        Runtime& operator=(Runtime&&) = delete;
        Runtime& operator=(const Runtime&) = delete;

        Domain& domain();

        template<typename RootTask, typename... Args>
        void spawn_thread(Args&&... args);

    private:
        void finalize(ObjectGroup group, std::span<Object*> objects) noexcept override;

    private:
        Domain                               domain_;
        Region                               region_;
        std::vector<std::unique_ptr<Thread>> threads_;
    };

    template<typename RootTask, typename... Args>
    void Runtime::spawn_thread(Args&&... args) {
        auto&& thread = threads_.emplace_back(std::make_unique<Thread>(*this));

        thread->run<RootTask>(std::forward<Args>(args)...);
    }

}
