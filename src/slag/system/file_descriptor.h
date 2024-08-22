#pragma once

#include <utility>
#include <unistd.h>
#include "slag/core.h"

namespace slag {

    template<>
    class Resource<ResourceType::FILE_DESCRIPTOR> : public Object {
    public:
        enum class Ownership {
            OWNED,
            BORROWED,
        };

        explicit Resource(int file_descriptor, Ownership ownership = Ownership::OWNED)
            : Object(static_cast<ObjectGroup>(ResourceType::FILE_DESCRIPTOR))
            , file_descriptor_(file_descriptor)
        {
            switch (ownership) {
                case Ownership::OWNED: {
                    file_descriptor_ = file_descriptor;
                    break;
                }
                case Ownership::BORROWED: {
                    file_descriptor_ = duplicate();
                    break;
                }
            }
        }

        ~Resource() {
            close();
        }

        explicit operator bool() const {
            return file_descriptor_ >= 0;
        }

        [[nodiscard]]
        int duplicate() const {
            if (file_descriptor_ >= 0) {
                int new_file_descriptor;

                do {
                    new_file_descriptor = dup(file_descriptor_);
                } while ((new_file_descriptor < 0) && (errno == EINTR));

                if (new_file_descriptor < 0) {
                    throw std::runtime_error(strerror(errno));
                }

                return new_file_descriptor;
            }

            return file_descriptor_;
        }

        int borrow() {
            return file_descriptor_;
        }

        // The reactor can 'crack' file descriptors to close them asynchronously.
        // Otherwise they will be automatically be closed synchronously.
        [[nodiscard]]
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

    template<typename... Args>
    inline Ref<FileDescriptor> make_file_descriptor(Args&&... args) {
        return bind(
            *new FileDescriptor(std::forward<Args>(args)...)
        );
    }

}
