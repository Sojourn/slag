#pragma once

#include <optional>
#include <vector>
#include <latch>
#include "mantle/mantle.h"
#include "types.h"

namespace slag {

    class Thread;

    class Application {
        Application(Application&&) = delete;
        Application(const Application&) = delete;
        Application& operator=(Application&&) = delete;
        Application& operator=(const Application&) = delete;

    public:
        Application(int argc, char** argv);

        template<typename RootTask, typename... RootTaskArgs>
        RootTask& spawn_thread(RootTaskArgs... root_task_args);

        int run();

    private:
        friend class Thread;

        mantle::Domain& domain();

        void handle_stopped(Thread& thread);

    private:
        mantle::Domain                       domain_;
        std::vector<std::unique_ptr<Thread>> threads_;
        std::optional<std::latch>            shutdown_latch_;
    };

    template<typename RootTask, typename... RootTaskArgs>
    RootTask& Application::spawn_thread(RootTaskArgs... root_task_args) {
        if (shutdown_latch_) {
            // TODO: Relax this restriction.
            throw std::runtime_error("Cannot spawn a thread after the application has started");
        }

        auto task = std::make_unique<RootTask>(std::forward<RootTaskArgs>(root_task_args)...);
        RootTask& task_ref = *task;
        threads_.push_back(std::make_unique<Thread>(*this, std::move(task)));
        return task_ref;
    }

}
