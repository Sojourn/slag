
#pragma once

#include <cstdint>
#include <cstddef>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include "slag/error.h"
#include "slag/result.h"

namespace slag {

    class Platform {
    public:
        static constexpr mode_t DEFAULT_OPEN_MODE = 0644;
        static constexpr int DEFAULT_SOCKET_PROTOCOL = 0;
        static constexpr int DEFAULT_LISTEN_BACKLOG = 4096;

        using VoidResult           = Result<void>;
        using ProcessIdResult      = Result<pid_t>;
        using FileDescriptorResult = Result<int>;

        Platform() = default;
        Platform(Platform&&) noexcept = delete;
        Platform(const Platform&) = delete;
        virtual ~Platform() = default;

        Platform& operator=(Platform&&) noexcept = delete;
        Platform& operator=(const Platform&) = delete;

        [[nodiscard]] virtual ProcessIdResult get_parent_process_id();
        [[nodiscard]] virtual ProcessIdResult get_process_id();
        [[nodiscard]] virtual ProcessIdResult get_thread_id();
        [[nodiscard]] virtual FileDescriptorResult open(const char* file_path, int flags, mode_t mode = DEFAULT_OPEN_MODE);
        [[nodiscard]] virtual VoidResult close(int file_descriptor);
        [[nodiscard]] virtual FileDescriptorResult duplicate(int file_descriptor);
        [[nodiscard]] virtual FileDescriptorResult socket(int domain, int type, int protocol = DEFAULT_SOCKET_PROTOCOL);
        [[nodiscard]] virtual VoidResult socketpair(int domain, int type, int protocol, int (&file_descriptors)[2]);
        [[nodiscard]] virtual VoidResult bind(int file_descriptor, const struct sockaddr* address, socklen_t address_length);
        [[nodiscard]] virtual VoidResult listen(int file_descriptor, int backlog = DEFAULT_LISTEN_BACKLOG);
    };

}
