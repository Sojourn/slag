#include "slag/platform.h"
#include <cerrno>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

auto slag::Platform::get_parent_process_id() -> ProcessIdResult {
    pid_t result = ::getppid();

    assert(result >= 0); // should always be successful
    if (result < 0) {
        return ProcessIdResult{make_system_error()};
    }

    return ProcessIdResult{result};
}

auto slag::Platform::get_process_id() -> ProcessIdResult {
    pid_t result = ::getpid();

    assert(result >= 0); // should always be successful
    if (result < 0) {
        return ProcessIdResult{make_system_error()};
    }

    return ProcessIdResult{result};
}

auto slag::Platform::get_thread_id() -> ProcessIdResult {
    pid_t result = ::gettid();

    assert(result >= 0); // should always be successful
    if (result < 0) {
        return ProcessIdResult{make_system_error()};
    }

    return ProcessIdResult{result};
}

auto slag::Platform::open(const char* file_path, int flags, mode_t mode) -> FileDescriptorResult {
    int result = 0;
    do {
        result = ::open(file_path, flags, mode);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        return FileDescriptorResult{make_system_error()};
    }

    return FileDescriptorResult{result};
}

auto slag::Platform::close(int file_descriptor) -> VoidResult {
    int result = 0;
    do {
        result = ::close(file_descriptor);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        return VoidResult{make_system_error()};
    }

    return VoidResult{};
}

auto slag::Platform::duplicate(int file_descriptor) -> FileDescriptorResult {
    int result = 0;
    do {
        result = ::dup(file_descriptor);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        return FileDescriptorResult{make_system_error()};
    }

    return FileDescriptorResult{result};
}

auto slag::Platform::socket(int domain, int type, int protocol) -> FileDescriptorResult {
    int result = 0;
    do {
        result = ::socket(domain, type, protocol);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        return FileDescriptorResult{make_system_error()};
    }

    return FileDescriptorResult{result};
}

auto slag::Platform::socketpair(int domain, int type, int protocol, int (&file_descriptors)[2]) -> VoidResult {
    int result = 0;
    do {
        result = ::socketpair(domain, type, protocol, file_descriptors);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        return VoidResult{make_system_error()};
    }

    return VoidResult{};
}

auto slag::Platform::bind(int file_descriptor, const struct sockaddr* address, socklen_t address_length) -> VoidResult {
    int result = 0;
    do {
        result = ::bind(file_descriptor, address, address_length);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        return VoidResult{make_system_error()};
    }

    return VoidResult{};
}

auto slag::Platform::listen(int file_descriptor, int backlog) -> VoidResult {
    int result = 0;
    do {
        result = ::listen(file_descriptor, backlog);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        return VoidResult{make_system_error()};
    }

    return VoidResult{};
}
