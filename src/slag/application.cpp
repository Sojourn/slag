#include "application.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Application::Application(int argc, char** argv)
        : region_(domain_, *this)
    {
        (void)argc;
        (void)argv;

        threads_.reserve(std::thread::hardware_concurrency());
    }

    Application::~Application() {
        region_.stop();
    }

    Domain& Application::domain() {
        return domain_;
    }

    void Application::finalize(ObjectGroup, std::span<Object*>) noexcept {
        abort(); // Override this if needed.
    }

}
