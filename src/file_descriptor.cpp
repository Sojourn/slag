#include "slag/file_descriptor.h"
#include "slag/event_loop.h"
#include "slag/util.h"
#include <utility>
#include <cerrno>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>

slag::FileDescriptor::FileDescriptor()
    : file_descriptor_{-1}
{
}

slag::FileDescriptor::FileDescriptor(int file_descriptor)
    : file_descriptor_{file_descriptor}
{
}

slag::FileDescriptor::FileDescriptor(FileDescriptor&& other) noexcept
    : file_descriptor_{other.release()}
{
}

slag::FileDescriptor::~FileDescriptor() {
    close();
}

slag::FileDescriptor& slag::FileDescriptor::operator=(FileDescriptor&& rhs) noexcept {
    if (this != &rhs) {
        close();
        file_descriptor_ = rhs.release();
    }

    return *this;
}

slag::FileDescriptor::operator bool() const {
    return is_open();
}

bool slag::FileDescriptor::is_open() const {
    return file_descriptor_ >= 0;
}

void slag::FileDescriptor::close() {
    if (is_open()) {
        int result = local_event_loop().platform().close(release());
        assert(result >= 0);
    }
}

int slag::FileDescriptor::borrow() {
    return file_descriptor_;
}

int slag::FileDescriptor::release() {
    return std::exchange(file_descriptor_, -1);
}

slag::FileDescriptor slag::FileDescriptor::duplicate() const {
    return duplicate(file_descriptor_);
}

slag::FileDescriptor slag::FileDescriptor::duplicate(int file_descriptor) {
    if (file_descriptor < 0) {
        return FileDescriptor{};
    }

    int new_file_descriptor = local_event_loop().platform().duplicate(file_descriptor);
    if (new_file_descriptor < 0) {
        raise_system_error("Failed to duplicate file descriptor");
    }

    return FileDescriptor{new_file_descriptor};
}
