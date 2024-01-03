#pragma once

#include <optional>
#include <vector>
#include <latch>
#include "types.h"

namespace slag {

    class Thread;

    // TODO:
    //   - Command line parameter parsing.
    //   - Global command queue.
    //   - Global coordination (epochs).
    //   - Component tree root.
    //   - Utility threads
    //      - Garbage collection.
    //      - Logging.
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

        ThreadIndex attach(Thread& thread);
        void detach(Thread& thread);

        void handle_stopped(Thread& thread);

    private:
        std::optional<std::latch> latch_;
        std::vector<Thread*>      threads_;
    };

}
