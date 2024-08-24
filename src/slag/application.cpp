#include "application.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    struct DummyFinalizer final : public Finalizer {
        void finalize(ObjectGroup, std::span<Object*>) noexcept override {
            abort();
        }
    };

    Application::Application(int argc, char** argv) {
        (void)argc;
        (void)argv;

        threads_.reserve(std::thread::hardware_concurrency());
    }

    Application::~Application() {
        // Create a dummy region if needed to prevent the domain from waiting forever.
        if (threads_.empty() && !Region::thread_local_instance()) {
            DummyFinalizer finalizer;
            Region region(domain_, finalizer);
        }
    }

    Domain& Application::domain() {
        return domain_;
    }

}
