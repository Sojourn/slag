#pragma once

#include <utility>
#include <unistd.h>
#include "slag/core.h"

namespace slag {

    template<>
    class Resource<ResourceType::FILE_DESCRIPTOR> : public Object {
    public:
        explicit Resource(int file_descriptor)
            : Object(static_cast<ObjectGroup>(ResourceType::FILE_DESCRIPTOR))
            , file_descriptor_(file_descriptor)
        {
        }

        ~Resource() {
            close();
        }

        int borrow() {
            return file_descriptor_;
        }

        // The reactor can 'crack' file descriptors to close them asynchronously.
        // Otherwise they will be automatically be closed synchronously.
        int release() {
            return std::exchange(file_descriptor_, -1);
        }

        void close() {
            if (file_descriptor_ >= 0) {
                int result = ::close(release());
                assert(result >= 0);
            }
        }

    private:
        int file_descriptor_;
    };

}
