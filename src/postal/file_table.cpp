#include "slag/postal/file_table.h"
#include "slag/postal/operation_factory.h"
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <sys/time.h>
#include <sys/resource.h>

namespace slag {

    static size_t query_max_file_descriptors() {
        struct rlimit resource_limit;
        memset(&resource_limit, 0, sizeof(resource_limit));

        int status = getrlimit(RLIMIT_NOFILE, &resource_limit);
        if (status < 0) {
            throw std::runtime_error("Failed to query the maximum number of file descriptors");
        }

        return static_cast<size_t>(resource_limit.rlim_max);
    }

    FileTable::FileTable() {
        entries_.resize(query_max_file_descriptors());
    }

    FileTable::~FileTable() {
        for (const Entry& entry: entries_) {
            assert(entry.reference_count == 0);
        }
    }

    void FileTable::increment_reference_count(int file_descriptor) {
        assert((0 <= file_descriptor) && (static_cast<size_t>(file_descriptor) < entries_.size()));

        Entry& entry = entries_[static_cast<size_t>(file_descriptor)];
        entry.reference_count += 1;
    }

    void FileTable::decrement_reference_count(int file_descriptor) {
        assert((0 <= file_descriptor) && (static_cast<size_t>(file_descriptor) < entries_.size()));

        Entry& entry = entries_[static_cast<size_t>(file_descriptor)];
        entry.reference_count -= 1;

        if (entry.reference_count == 0) {
            make_close_operation(file_descriptor)->daemonize();
        }
    }

}
