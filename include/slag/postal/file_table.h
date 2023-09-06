#pragma once

#include <vector>

namespace slag::postal {

    class Region;

    class FileTable {
        FileTable(FileTable&&) = delete;
        FileTable(const FileTable&) = delete;
        FileTable& operator=(FileTable&&) = delete;
        FileTable& operator=(const FileTable&) = delete;

    public:
        FileTable();
        ~FileTable();

    private:
        friend class FileHandle;

        void increment_reference_count(int file_descriptor);
        void decrement_reference_count(int file_descriptor);

    private:
        struct Entry {
            size_t reference_count = 0;
        };

        std::vector<Entry> entries_;
    };

}
