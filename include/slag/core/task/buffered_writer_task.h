#pragma once

#include <limits>
#include <chrono>
#include <variant>
#include <cstdint>
#include <cstddef>
#include "slag/token_bucket.h"
#include "slag/core/task/proto_task.h"
#include "slag/core/pollable.h"
#include "slag/core/pollable/pollable_queue.h"
#include "slag/system.h"

namespace slag {

    // TODO: think about having multiple in-flight write operations at once. We
    //       know buffer and file offsets, so we should be able to kick them off
    //       from BufferWriterTask::write, potentially with a fixed concurrency limit.
    //
    class BufferWriterTask
        : public ProtoTask
        , public Pollable<PollableType::WRITABLE> 
    {
    public:
        explicit BufferWriterTask(FileHandle file);

        // Returns true if this task is currently idle.
        bool is_quiescent() const;

        // Setup or configure a token bucket to enforce these rate limiting parameters.
        // Default: no rate limiting (unlimited).
        void set_rate_limit(uint64_t bytes_per_second, uint64_t burst_capacity);

        // Set the level below which this will advertise that it is writable.
        // Default: always writable.
        void set_watermark(size_t watermark);

        // Append additional data which will be written to the file in the background.
        void write(BufferHandle buffer);

        // Wait for any remaining data to be flushed before completing successfully.
        // Avoid writing additional data after calling shutdown.
        void shutdown();

    public:
        void run() override final;

        Event& writable_event() override final;

    private:
        size_t pending_write_size() const;

        WriteOperationHandle& write_operation();
        TimerOperationHandle& timer_operation();

        Result<size_t> consume_write_operation();
        Result<void> consume_timer_operation();
        void reset_operation();

        bool handle_write_complete();
        bool handle_timer_complete();

        void update_token_bucket();
        void update_readiness();

    private:
        FileHandle                  file_;
        uint64_t                    file_offset_;
        size_t                      buffer_offset_;

        PollableQueue<BufferHandle> pending_data_;
        size_t                      pending_size_bytes_;
        std::optional<TokenBucket>  token_bucket_;

        uint64_t                    wait_nanoseconds_;
        Event                       writable_event_;
        size_t                      watermark_;
        bool                        shutting_down_;

        std::variant<
            std::monostate,
            WriteOperationHandle,
            TimerOperationHandle
        > operation_;
    };

}
