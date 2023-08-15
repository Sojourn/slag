#pragma once

#include <chrono>
#include <utility>
#include <iostream>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

namespace slag {
    template<typename... Args>
    inline void info(const char* format, Args&&... args) {
        std::cout << "[Info]  " << fmt::format(fmt::runtime(format), std::forward<Args>(args)...) << std::endl;
    }

    template<typename... Args>
    inline void error(const char* format, Args&&... args) {
        std::cout << "[Error] " << fmt::format(fmt::runtime(format), std::forward<Args>(args)...) << std::endl;
    }

    template<typename... Args>
    inline void fatal(const char* format, Args&&... args) {
        std::cout << "[Fatal] " << fmt::format(fmt::runtime(format), std::forward<Args>(args)...) << std::endl;
    }

    template<typename... Args>
    inline void trace(const char* format, Args&&... args) {
        (void)format;
        ((void)args, ...);

        // std::cout << fmt::format(fmt::runtime(format), std::forward<Args>(args)...) << std::endl;
    }
}
