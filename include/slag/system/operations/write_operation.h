#pragma once

#include <array>
#include <string>
#include <cerrno>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <sys/uio.h>
#include "slag/postal/file_handle.h"
#include "slag/postal/buffer_ledger.h"
#include "slag/core/service_interface.h"
#include "slag/system/primitive_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::WRITE> : public PrimitiveOperation<size_t> {
    public:
        using BufferEntry = NationalBufferLedgerEntry;

        explicit Operation(SystemServiceInterface& system_service, FileHandle file, uint64_t file_offset, BufferHandle buffer, size_t buffer_offset)
            : PrimitiveOperation<size_t>{OperationType::WRITE, system_service}
            , file_{std::move(file)}
            , file_offset_{file_offset}
            , buffer_{std::move(buffer)}
            , buffer_offset_{buffer_offset}
        {
            assert(file_);
            assert(buffer_);
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            const BufferEntry&   buffer_entry = buffer_.national_entry();
            const BufferSegment* buffer_segment = &buffer_entry.head;

            // Calculate how many bytes we want to try writing.
            size_t remainder = buffer_entry.size;
            if (buffer_offset_ < remainder) {
                remainder -= buffer_offset_;
            }

            // Populate the iovec array.
            size_t iov_index = 0;
            size_t current_buffer_offset = 0;
            while (remainder && (iov_index < iov_array_.size())) {
                if (buffer_offset_ < (current_buffer_offset + buffer_segment->capacity)) {
                    size_t iov_len = std::min(
                        buffer_segment->capacity,
                        remainder
                    );

                    struct iovec& iov = iov_array_[iov_index++];
                    iov.iov_base = buffer_segment->data;
                    iov.iov_len = iov_len;

                    remainder -= iov_len;
                }

                current_buffer_offset += buffer_segment->capacity;
                buffer_segment = buffer_segment->next;
            }

            io_uring_prep_writev(
                &sqe,
                file_.file_descriptor(),
                iov_array_.data(),
                iov_index,
                file_offset_
            );
        }

        Result<size_t> handle_operation_result(int32_t result, bool more) override final {
            assert(!more);

            if (result < 0) {
                return make_system_error(-result);
            }

            return Result<size_t> {
                static_cast<size_t>(result)
            };
        }

    private:
        FileHandle                  file_;
        uint64_t                    file_offset_;
        BufferHandle                buffer_;
        size_t                      buffer_offset_;
        std::array<struct iovec, 8> iov_array_;
    };

}
