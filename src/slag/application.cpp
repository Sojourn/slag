#include "application.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Application::Application(int argc, char** argv) {
        (void)argc;
        (void)argv;

        threads_.reserve(std::thread::hardware_concurrency());
    }

    Domain& Application::domain() {
        return domain_;
    }

    int Application::run() {
        if (shutdown_latch_) {
            throw std::runtime_error("Application is already running");
        }
        if (threads_.empty()) {
            throw std::runtime_error("Application must contain at least one thread");
        }

        // Start threads and wait for them to complete.
        {
            shutdown_latch_.emplace(static_cast<std::ptrdiff_t>(threads_.size()));

            for (auto&& thread: threads_) {
                if (thread) {
                    thread->start();
                }
                else {
                    shutdown_latch_->count_down();
                }
            }

            shutdown_latch_->wait();
        }

        return EXIT_SUCCESS;
    }

    void Application::handle_stopped(Thread&) {
        assert(shutdown_latch_);

        shutdown_latch_->count_down();
    }

}
