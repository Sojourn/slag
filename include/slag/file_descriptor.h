#pragma once

namespace slag {

    class FileDescriptor {
    public:
        FileDescriptor();
        explicit FileDescriptor(int file_descriptor);
        FileDescriptor(FileDescriptor&& other) noexcept;
        FileDescriptor(const FileDescriptor&) = delete;
        ~FileDescriptor();

        FileDescriptor& operator=(FileDescriptor&& rhs) noexcept;
        FileDescriptor& operator=(const FileDescriptor&) = delete;

        explicit operator bool() const;
        [[nodiscard]] bool is_open() const;
        void close();

        [[nodiscard]] int borrow();
        [[nodiscard]] int release();

        [[nodiscard]] FileDescriptor duplicate() const;
        static [[nodiscard]] FileDescriptor duplicate(int file_descriptor);

    private:
        int file_descriptor_;
    };

}
