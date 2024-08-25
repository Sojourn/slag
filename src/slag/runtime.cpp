#include "runtime.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Runtime::Runtime(int argc, char** argv)
        : region_(domain_, *this)
    {
        (void)argc;
        (void)argv;

        threads_.reserve(std::thread::hardware_concurrency());
    }

    Runtime::~Runtime() {
        region_.stop();
    }

    Domain& Runtime::domain() {
        return domain_;
    }

    void Runtime::finalize(ObjectGroup, std::span<Object*>) noexcept {
        abort(); // Override this if needed.
    }

}
