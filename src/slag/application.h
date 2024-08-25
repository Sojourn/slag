#pragma once

#include <optional>
#include <vector>
#include <latch>
#include "slag/core.h"
#include "slag/types.h"
#include "thread.h"

namespace slag {

    class Thread;

    class Application : public Finalizer {
    public:
        Application(int argc, char** argv);
        ~Application();

        Application(Application&&) = delete;
        Application(const Application&) = delete;
        Application& operator=(Application&&) = delete;
        Application& operator=(const Application&) = delete;

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
    void Application::spawn_thread(Args&&... args) {
        auto&& thread = threads_.emplace_back(std::make_unique<Thread>(*this));

        thread->run<RootTask>(std::forward<Args>(args)...);
    }

}
