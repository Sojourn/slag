#pragma once

#include <limits>
#include <cstdint>
#include <cstddef>
#include "slag/types.h"

namespace slag {

    constexpr size_t MAX_THREAD_COUNT = 32;
    constexpr size_t INVALID_THREAD_INDEX = MAX_THREAD_COUNT;
    static_assert(MAX_THREAD_COUNT <= std::numeric_limits<decltype(ThreadIndex)>::max());

    constexpr size_t MAX_BUFFER_COUNT = 1 << 20;
    constexpr size_t INVALID_BUFFER_INDEX = MAX_BUFFER_COUNT;
    static_assert(MAX_BUFFER_COUNT <= std::numeric_limits<decltype(BufferIndex)>::max());

}
