#pragma once

#include "slag/core/service_interface.h"

namespace slag {

    class FileHandle {
    public:
        FileHandle();
        explicit FileHandle(int file_descriptor);
        ~FileHandle();

        FileHandle(FileHandle&& other);
        FileHandle(const FileHandle& other);
        FileHandle& operator=(FileHandle&& that);
        FileHandle& operator=(const FileHandle& that);

        explicit operator bool() const;

        int file_descriptor();

        void reset();

    private:
        static void increment_reference_count(int file_descriptor);
        static void decrement_reference_count(int file_descriptor);

    private:
        int file_descriptor_;
    };

}
