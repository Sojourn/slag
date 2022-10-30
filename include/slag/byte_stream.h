#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "slag/handle.h"

namespace slag {

    // TODO: make this better and move it into a dedicated file
    class Buffer {
    public:
        Buffer(size_t capacity) {
            data_.resize(capacity);
        }

        Buffer(std::span<const std::byte> data) {
            data_.insert(data_.end(), data.begin(), data.end());
        }

        [[nodiscard]] std::span<std::byte> data() {
            return std::span{data_.data(), data_.size()};
        }

        [[nodiscard]] std::span<const std::byte> data() const {
            return std::span{data_.data(), data_.size()};
        }

    private:
        std::vector<std::byte> data_;
    };

    class ByteStream {
    public:
        // lifetime of the result lasts until the returned buffer handle is discarded.
        [[nodiscard]] std::pair<std::span<const std::byte>, Handle<Buffer>> read_stable(size_t count) {
            for (auto segment_it = segments_.begin(); segment_it != segments_.end(); ) {
                auto data = segment_it->consumer_data();
                if (data.empty()) {
                    // we leave fully consumed segments for temporary stability
                    segment_it = segments_.erase(segment_it);
                    continue;
                }

                count = std::min(count, data.size_bytes());
                data = data.last(count);

                segment_it->relative_consumer_sequence += count;
                absolute_consumer_sequence_ += count;

                return std::make_pair(data, segment_it->buffer);
            }

            return std::make_pair(std::span<const std::byte>{}, Handle<Buffer>{});
        }

        [[nodiscard]] size_t readable_byte_count() const {
            return absolute_producer_sequence_ - absolute_consumer_sequence_;
        }

        // lifetime of the result lasts until the next consumer action.
        [[nodiscard]] std::span<const std::byte> read(size_t count) {
            auto&& [result, buffer] = read_stable(count);
            (void)buffer; 
            return result;
        }

        // undoes the previous read
        [[nodiscard]] size_t unread(size_t count) {
            size_t remainder = count;
            for (auto it = segments_.rbegin(); it != segments_.rend(); ++it) {
                size_t relative_count = std::min(it->relative_consumer_sequence, remainder);

                it->relative_consumer_sequence -= relative_count;
                absolute_consumer_sequence_    -= relative_count;
                remainder                      -= relative_count;

                if (remainder == 0) {
                    break; // early-exit
                }
            }

            return count - remainder;
        }

        [[nodiscard]] std::pair<std::span<const std::byte>, Handle<Buffer>> peek_stable(size_t count) {
            auto result = read_stable(count);
            if (!result.first.empty()) {
                size_t bytes_unread = unread(result.first.size_bytes());
                assert(bytes_unread == result.first.size_bytes());
            }

            return result;
        }

        [[nodiscard]] std::span<const std::byte> peek(size_t count) {
            auto&& [result, buffer] = peek_stable(count);
            (void)buffer;
            return result;
        }

        void write(std::span<const std::byte> data, Handle<Buffer> buffer = {}) {
            // NOTE: prefer to create a new segment if a buffer is provided instead of copying
            if (buffer) {
                segments_.push_back(Segment {
                    .buffer                     = std::move(buffer),
                    .frozen                     = true,
                    .relative_consumer_sequence = 0,
                    .relative_producer_sequence = data.size_bytes(),
                });
            }
            else {
                if (!segments_.empty()) {
                    // TODO: check if the last segment has room for the data
                }

                // create a new buffer for the remaining data
                buffer = make_handle<Buffer>(data);
                segments_.push_back(Segment {
                    .buffer                     = std::move(buffer),
                    .frozen                     = false,
                    .relative_consumer_sequence = 0,
                    .relative_producer_sequence = data.size_bytes(),
                });
            }

            absolute_producer_sequence_ += data.size_bytes();
        }

    private:
        struct Segment {
            Handle<Buffer> buffer;
            bool           frozen                     = false;
            uint64_t       relative_consumer_sequence = 0;
            uint64_t       relative_producer_sequence = 0;

            std::span<std::byte> consumer_data() {
                auto data = buffer->data();
                return data.subspan(
                    relative_consumer_sequence,
                    relative_producer_sequence - relative_consumer_sequence
                );
            }

            std::span<std::byte> producer_data() {
                auto data = buffer->data();
                return data.subspan(
                    relative_producer_sequence,
                    data.size_bytes() - relative_producer_sequence
                );
            }
        };

        uint64_t             absolute_producer_sequence_ = 0;
        uint64_t             absolute_consumer_sequence_ = 0;
        std::vector<Segment> segments_;
    };

}
