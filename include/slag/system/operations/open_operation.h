#pragma once

#include <string>
#include <cerrno>
#include <cassert>
#include "slag/postal/file_handle.h"
#include "slag/core/service_interface.h"
#include "slag/system/primitive_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::OPEN> : public PrimitiveOperation<FileHandle> {
    public:
        static constexpr mode_t DEFAULT_OPEN_MODE_ = 0644;

        // Opens a file relative to the current working directory.
        explicit Operation(SystemServiceInterface& system_service, const std::string& file_path, int flags, mode_t mode = DEFAULT_OPEN_MODE_)
            : PrimitiveOperation{OperationType::OPEN, system_service}
            , file_path_{std::move(file_path)}
            , flags_{flags}
            , mode_{mode}
        {
        }

        // Opens a file relative to a directory.
        explicit Operation(SystemServiceInterface& system_service, FileHandle directory, std::string file_path, int flags, mode_t mode = DEFAULT_OPEN_MODE_)
            : PrimitiveOperation{OperationType::OPEN, system_service}
            , directory_{directory}
            , file_path_{std::move(file_path)}
            , flags_{flags}
            , mode_{mode}
        {
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            io_uring_prep_openat(
                &sqe,
                directory_ ? directory_->file_descriptor() : AT_FDCWD,
                file_path_.c_str(),
                flags_,
                mode_
            );
        }

        Result<FileHandle> handle_operation_result(int32_t result, bool more) override final {
            assert(!more);

            if (result < 0) {
                return make_system_error(-result);
            }
            
            return Result<FileHandle>(FileHandle(result));
        }

    private:
        std::optional<FileHandle> directory_;
        std::string               file_path_;
        int                       flags_;
        mode_t                    mode_;
    };

}
