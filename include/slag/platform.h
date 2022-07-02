#pragma once

#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace slag {

    class Platform {
    public:
        virtual ~Platform() = default;

        virtual [[nodiscard]] int get_process_id();
        virtual [[nodiscard]] int get_thread_id();
        virtual [[nodiscard]] int open(const char* file_path, int flags, mode_t mode = 0644);
        virtual [[nodiscard]] int close(int file_descriptor);
        virtual [[nodiscard]] int duplicate(int file_descriptor);
    };

}
