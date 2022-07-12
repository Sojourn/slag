#pragma once

#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace slag {

    class Platform {
    public:
        Platform() = default;
        Platform(Platform&&) noexcept = delete;
        Platform(const Platform&) = delete;
        ~Platform() = default;

        Platform& operator=(Platform&&) noexcept = delete;
        Platform& operator=(const Platform&) = delete;

        [[nodiscard]] int get_parent_process_id();
        [[nodiscard]] int get_process_id();
        [[nodiscard]] int get_thread_id();
        [[nodiscard]] int open(const char* file_path, int flags, mode_t mode = 0644);
        [[nodiscard]] int close(int file_descriptor);
        [[nodiscard]] int duplicate(int file_descriptor);
    };

}
