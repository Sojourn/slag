#include "application.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Application::Application(int argc, char** argv) {
        (void)argc;
        (void)argv;
    }

    int Application::run() {
        if (latch_) {
            throw std::runtime_error("Application is already running");
        }
        if (threads_.empty()) {
            throw std::runtime_error("Application must contain at least one thread");
        }

        // Start threads and wait for them to complete.
        {
            latch_.emplace(static_cast<std::ptrdiff_t>(threads_.size()));

            for (Thread* thread: threads_) {
                if (thread) {
                    thread->start();
                }
                else {
                    latch_->count_down();
                }
            }

            latch_->wait();
        }

        return EXIT_SUCCESS;
    }

    mantle::Domain& Application::domain() {
        return domain_;
    }

    ThreadIndex Application::attach(Thread& thread) {
        if (latch_) {
            throw std::runtime_error("Threads cannot be added to a running application");
        }

        ThreadIndex thread_index = threads_.size();
        threads_.push_back(&thread);
        return thread_index;
    }

    void Application::detach(Thread& thread) {
        threads_.at(thread.index()) = nullptr;
    }

    void Application::handle_stopped(Thread&) {
        assert(latch_);

        latch_->count_down();
    }

}
