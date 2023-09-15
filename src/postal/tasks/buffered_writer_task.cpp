#include "slag/postal/tasks/buffered_writer_task.h"
#include <cassert>

namespace slag::postal {

    BufferWriterTask::BufferWriterTask(FileHandle file)
        : file_{std::move(file)}
        , file_offset_{0}
        , buffer_offset_{0}
        , pending_size_bytes_{0}
        , wait_nanoseconds_{0}
        , watermark_{std::numeric_limits<size_t>::max()}
        , shutting_down_{false}
    {
        writable_event_.set();
    }

    bool BufferWriterTask::is_quiescent() const {
        return pending_size_bytes_ == 0;
    }

    void BufferWriterTask::set_rate_limit(uint64_t bytes_per_second, uint64_t burst_capacity) {
        if (token_bucket_) {
            token_bucket_->configure(bytes_per_second, burst_capacity);
        }
        else {
            token_bucket_.emplace(bytes_per_second, burst_capacity);
        }
    }

    void BufferWriterTask::set_watermark(size_t watermark) {
        watermark_ = watermark;

        update_readiness();
    }

    void BufferWriterTask::write(BufferHandle buffer) {
        assert(!is_complete());
        assert(!shutting_down_);
        assert(buffer);

        size_t size = buffer.size();
        if (size == 0) {
            return;
        }

        pending_data_.push_back(std::move(buffer));
        pending_size_bytes_ += size;
    }

    void BufferWriterTask::shutdown() {
        shutting_down_ = true;

        // Immediate.
        if (is_quiescent()) {
            set_success();
        }
    }

    void BufferWriterTask::run() {
        SLAG_PT_BEGIN();

        while (true) {
            SLAG_PT_WAIT_READABLE(pending_data_);
            while (pending_data_.is_empty()) {
                if (token_bucket_) {
                    while (true) {
                        // Try to consume tokens for this write, or wait until we are able to.
                        wait_nanoseconds_ = token_bucket_->consume(pending_write_size());
                        if (wait_nanoseconds_ == TokenBucket::NO_WAIT_NS) {
                            break;
                        }
                        else if (wait_nanoseconds_ == TokenBucket::INFINITE_WAIT_NS) {
                            set_failure(); // MTU error.
                            return;
                        }
                        else {
                            // Wait the requested number of nanoseconds before updating token counts.
                            operation_ = make_timer_operation(
                                std::chrono::nanoseconds(wait_nanoseconds_)
                            );
                            SLAG_PT_WAIT_COMPLETE(*timer_operation());
                            handle_write_complete();
                            update_token_bucket();
                        }
                    }
                }

                // Kick off a write operation for the remainder of the next pending buffer.
                operation_ = make_write_operation(
                    file_,
                    file_offset_,
                    pending_data_.front().share(),
                    buffer_offset_
                );
                SLAG_PT_WAIT_COMPLETE(*write_operation());
                handle_write_complete();
            }

            // Check if we have finished gracefully shutting down.
            if (shutting_down_ && is_quiescent()) {
                set_success();
                return;
            }
        }

        SLAG_PT_END();
    }

    Event& BufferWriterTask::writable_event() {
        return writable_event_;
    }

    size_t BufferWriterTask::pending_write_size() const {
        if (pending_data_.is_empty()) {
            return 0;
        }

        return pending_data_.front().size() - buffer_offset_;
    }

    WriteOperationHandle& BufferWriterTask::write_operation() {
        return std::get<WriteOperationHandle>(operation_);
    }

    TimerOperationHandle& BufferWriterTask::timer_operation() {
        return std::get<TimerOperationHandle>(operation_);
    }

    Result<size_t> BufferWriterTask::consume_write_operation() {
        auto result = write_operation()->result();
        reset_operation();
        return result;            
    }

    Result<void> BufferWriterTask::consume_timer_operation() {
        auto result = timer_operation()->result();
        reset_operation();
        return result;            
    }

    void BufferWriterTask::reset_operation() {
        operation_ = std::monostate{};
    }

    bool BufferWriterTask::handle_write_complete() {
        auto result = consume_write_operation();
        if (!result) {
            set_failure();
            return false;
        }

        size_t bytes_written = result.value();
        BufferHandle& buffer = pending_data_.front();

        if ((buffer_offset_ + bytes_written) == buffer.size()) {
            pending_data_.consume_front();
            buffer_offset_ = 0;
        }
        else {
            buffer_offset_ += bytes_written;
        }

        file_offset_ += bytes_written;
        pending_size_bytes_ -= bytes_written;
        update_readiness();

        return true;
    }

    bool BufferWriterTask::handle_timer_complete() {
        auto result = consume_timer_operation();
        if (!result) {
            set_failure();
            return false;
        }

        return true;
    }

    void BufferWriterTask::update_token_bucket() {
        auto now_time_point = std::chrono::steady_clock::now();
        auto now_duration = now_time_point.time_since_epoch();

        token_bucket_->update(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now_duration).count()
        );
    }

    void BufferWriterTask::update_readiness() {
        writable_event_.set(pending_size_bytes_ < watermark_);
    }

}
