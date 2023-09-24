#include "slag/postal/file_handle.h"
#include "slag/core/domain.h"
#include <utility>
#include <cassert>

namespace slag {

    FileHandle::FileHandle()
        : file_descriptor_{-1}
    {
    }

    FileHandle::FileHandle(int file_descriptor)
        : file_descriptor_{file_descriptor}
    {
        if (*this) {
            region().file_table().increment_reference_count(file_descriptor_);
        }
    }

    FileHandle::~FileHandle() {
        reset();
    }

    FileHandle::FileHandle(FileHandle&& other)
        : FileHandle{}
    {
        std::swap(file_descriptor_, other.file_descriptor_);
    }

    FileHandle::FileHandle(const FileHandle& other)
        : FileHandle{other.file_descriptor_}
    {
    }

    FileHandle& FileHandle::operator=(FileHandle&& that) {
        if (this != &that) {
            reset();

            std::swap(file_descriptor_, that.file_descriptor_);
        }

        return *this;
    }

    FileHandle& FileHandle::operator=(const FileHandle& that) {
        if (this != &that) {
            reset();

            if (that) {
                file_descriptor_ = that.file_descriptor_;
                region().file_table().increment_reference_count(
                    file_descriptor_
                );
            }
        }

        return *this;
    }

    FileHandle::operator bool() const {
        return file_descriptor_ >= 0;
    }

    int FileHandle::file_descriptor() {
        return file_descriptor_;
    }

    void FileHandle::reset() {
        if (*this) {
            region().file_table().decrement_reference_count(
                std::exchange(file_descriptor_, -1)
            );
        }
    }

}
