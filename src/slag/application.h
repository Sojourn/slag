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

        int run();

    private:
        friend class Thread;

        mantle::Domain& domain();

        ThreadIndex attach(Thread& thread);
        void detach(Thread& thread);

        void handle_stopped(Thread& thread);

    private:
        mantle::Domain            domain_;
        std::optional<std::latch> latch_;
        std::vector<Thread*>      threads_;
    };

}
