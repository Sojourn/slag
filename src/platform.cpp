#include "slag/platform.h"
#include <cerrno>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int slag::Platform::get_parent_process_id() {
    pid_t pid = ::getppid();
    assert(pid >= 0); // should always be successful
    return static_cast<int>(pid);
}

int slag::Platform::get_process_id() {
    pid_t pid = ::getpid();
    assert(pid >= 0); // should always be successful
    return static_cast<int>(pid);
}

int slag::Platform::get_thread_id() {
    pid_t pid = ::gettid();
    assert(pid >= 0); // should always be successful
    return static_cast<int>(pid);
}

int slag::Platform::open(const char* file_path, int flags, mode_t mode) {
    int result = 0;
    do {
        result = ::open(file_path, flags, mode);
    } while ((result < 0) && (errno == EINTR));

    return result;
}

int slag::Platform::close(int file_descriptor) {
    int result = 0;
    do {
        result = ::close(file_descriptor);
    } while ((result < 0) && (errno == EINTR));

    return result;
}

int slag::Platform::duplicate(int file_descriptor) {
    int result = 0;
    do {
        result = ::dup(file_descriptor);
    } while ((result < 0) && (errno == EINTR));

    return result;
}

int slag::Platform::socket(int domain, int type, int protocol) {
    int result = 0;
    do {
        result = ::socket(domain, type, protocol);
    } while ((result < 0) && (errno == EINTR));

    return result;
}

int slag::Platform::bind(int file_descriptor, const struct sockaddr* address, socklen_t address_length) {
    int result = 0;
    do {
        result = ::bind(file_descriptor, address, address_length);
    } while ((result < 0) && (errno == EINTR));

    return result;
}

int slag::Platform::listen(int file_descriptor, int backlog) {
    int result = 0;
    do {
        result = ::listen(file_descriptor, backlog);
    } while ((result < 0) && (errno == EINTR));

    return result;
}
