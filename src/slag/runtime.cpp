#include "runtime.h"
#include "thread.h"
#include "mantle/mantle.h"
#include <cstdlib>

namespace slag {

    Runtime::Runtime(const ThreadGraph& thread_graph)
        : region_(domain_, *this)
        , thread_graph_(thread_graph)
    {
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
