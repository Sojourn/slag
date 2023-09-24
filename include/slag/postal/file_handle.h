#pragma once

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
        int file_descriptor_;
    };

}
