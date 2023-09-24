#pragma once

#include <cerrno>
#include <cassert>
#include "slag/postal/primitive_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::TIMER> : public PrimitiveOperation<void> {
    public:
        template<typename Rep, typename Period>
        Operation(Reactor& reactor, const std::chrono::duration<Rep, Period>& duration)
            : PrimitiveOperation{OperationType::TIMER, reactor}
            , timespec_{to_kernel_timespec(duration)}
        {
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            int count = 0; // disabled
            int flags = IORING_TIMEOUT_BOOTTIME | IORING_TIMEOUT_ETIME_SUCCESS;

            io_uring_prep_timeout(&sqe, &timespec_, count, flags);
        }

        Result<void> handle_operation_result(int32_t result, bool more) override final {
            assert(!more);

            if (result < 0) {
                switch (-result) {
                    case ETIME: {
                        // I don't think we should be getting here with IORING_TIMEOUT_ETIME_SUCCESS,
                        // but here we are...
                        break;
                    }
                    default: {
                        return make_system_error(-result);
                    }
                }
            }

            return {};
        }

    private:
        static struct __kernel_timespec to_kernel_timespec(const std::chrono::nanoseconds& duration) {
            struct __kernel_timespec kernel_timespec;
            kernel_timespec.tv_sec = duration.count() / 1000000000;
            kernel_timespec.tv_nsec = duration.count() % 1000000000;
            return kernel_timespec;
        }

        template<typename Rep, typename Period>
        static struct __kernel_timespec to_kernel_timespec(const std::chrono::duration<Rep, Period>& duration) {
            return to_kernel_timespec(
                std::chrono::duration_cast<std::chrono::nanoseconds>(duration)
            );
        }

    private:
        struct __kernel_timespec timespec_;
    };

}
