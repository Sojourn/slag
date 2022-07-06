#pragma once

#include <chrono>
#include <utility>
#include <iostream>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

namespace slag {
    template<typename... Args>
    void info(const char* format, Args&&... args) {
        std::cout << fmt::format(fmt::runtime(format), std::forward<Args>(args)...) << std::endl;
    }
}
