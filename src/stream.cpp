#include "slag/stream.h"
#include "slag/util.h"
#include <type_traits>
#include <limits>
#include <cassert>

const char* slag::stream_transaction_aborted::what() const noexcept {
    return "stream transaction aborted";
}

slag::stream::stream(size_t minimum_capacity)
    : buffer_{std::make_shared<stream_buffer>(minimum_capacity)}
    , producer_sequence_{0}
    , consumer_sequence_{0}
{
}

slag::stream::~stream() {
    for (stream_producer* producer: producers_) {
        producer->abandon();
    }
    for (stream_consumer* consumer: consumers_) {
        consumer->abandon();
    }
}

stream_buffer& buffer() {
    return *buffer_;
}

const stream_buffer& buffer() const {
    return *buffer_;
}

std::shared_ptr<stream_buffer> shared_buffer() {
    return buffer_;
}

const std::shared_ptr<stream_buffer>& shared_buffer() const {
    return buffer_;
}

std::span<std::byte> slag::stream::producer_segment() {
    return buffer_->make_span(producer_sequence_, producer_segment_size());
}

void slag::stream::resize_producer_segment(size_t minimum_size) {
    if (minimum_size <= producer_segment_size()) {
        return; // segment already meets the minimum size
    }

    // allocate a new buffer large enough to hold the consumer segment and a producer segment
    // of at least minimum_size
    size_t copy_size = consumer_segment_size();
    std::shared_ptr<stream_buffer> new_buffer = std::make_shared<stream_buffer>(copy_size + minimum_size)

    // copy the consumer segment
    std::span<const std::byte> src = buffer_->make_span(consumer_segment_, copy_size);
    std::span<std::byte> dst = new_buffer->make_span(consumer_segment_, copy_size);
    memcpy(dst.data(), src.data(), copy_size);

    std::exchange(buffer_, new_buffer);
}

void slag::stream::advance_producer_sequence(size_t byte_count) {
    assert(byte_count <= producer_segment_size())
    producer_sequence_ += byte_count;
}

void slag::stream::add_producer(stream_producer& producer) {
    assert(!std::count(producers_.begin(), producers_.end(), &producer));
    producers_.push_back(&producer);
}

void slag::stream::remove_producer(stream_producer& producer) {
    bool removed = swap_and_swap(producers_, &producer);
    assert(removed);
}

size_t slag::stream::active_producer_transaction_count() const {
    size_t count = 0;
    for (const stream_producer* producer: producers_) {
        if (producer->has_active_transaction()) {
            count += 1;
        }
    }

    return count;
}

size_t slag::stream::consumer_segment_size() const {
    return producer_sequence_ - consumer_sequence_;
}

std::span<const std::byte> slag::stream::consumer_segment() const {
    return buffer_->make_span(consumer_sequence_, consumer_segment_size);
}

void slag::stream::update_consumer_sequence() {
    stream_sequence minimum_consumer_sequence = std::numeric_limits<stream_sequence>::max();
    for (const stream_consumer* consumer: consumers_) {
        minimum_consumer_sequence = std::min(consumer->sequence(), minimum_consumer_sequence);
    }

    if (consumer_sequence_ < minimum_consumer_sequence) {
        consumer_sequence_ = minimum_consumer_sequence;
    }
}

void slag::stream::add_consumer(stream_consumer& consumer) {
    assert(!std::count(consumers_.begin(), consumers_.end(), &consumer));
    consumers_.push_back(&consumer);
}

void slag::stream::remove_consumer(stream_consumer& consumer) {
    bool removed = swap_and_swap(consumers_, &consumer);
    assert(removed);
}

size_t slag::stream::active_consumer_transaction_count() const {
    size_t count = 0;
    for (const stream_consumer* consumer: consumers_) {
        if (consumer->has_active_transaction()) {
            count += 1;
        }
    }

    return count;
}

slag::stream_producer_transaction::stream_producer_transaction(stream_producer& producer)
    : producer_{&producer}
    , state_{stream_transaction_state::ACTIVE}
    , 
{
}

slag::stream_producer_transaction::stream_producer_transaction()
    : producer_{nullptr}
    , state_{stream_transaction_state::ABORTED}
    , producer_sequence_{0}
{
}

slag::stream_producer_transaction::stream_producer_transaction(stream_producer_transaction&& other) {
}

slag::stream_producer_transaction::stream_producer_transaction& operator=(stream_producer_transaction&& other) {
    if (this != &rhs) {
        if (producer_) {
            producer_->link(*this);
        }

        producer_ = rhs.producer_;
        state_ = rhs.state_;
        producer_sequence_ = rhs.producer_sequence_;
        buffer_ = rhs.buffer_;

        if (producer_) {
            producer_->reattach(*this);
        }
    }

    return *this;
}

slag::stream_producer_transaction::~stream_producer_transaction() {
    if (state_ == stream_transaction_state::ACTIVE) {
        rollback(); // uncommitted transactions are rolled back by default
    }
}

slag::stream_transaction_state slag::stream_producer_transaction::state() const {
    return state_;
}

bool slag::stream_producer_transaction::is_active() const {
    return state_ == stream_transaction_state::ACTIVE;
}

void slag::stream_producer_transaction::rollback() {
    if (!is_active()) {
        throw stream_transaction_aborted;
    }

    // TODO
}

void slag::stream_producer_transaction::commit() {
    if (!is_active()) {
        throw stream_transaction_aborted;
    }

    // TODO
}

void slag::stream_producer_transaction::write(std::span<const std::byte> buffer) {
    if (!is_active()) {
        throw stream_transaction_aborted;
    }

    size_t len = buffer.size_bytes();
    if (!len) {
        return true; // no-op
    }

    resize(len);

    std::byte* dst_buf = data().data();
    const std::byte* src_buf = buffer.data();
    memcpy(dst_buf, src_buf, len);
}

std::span<std::byte> slag::stream_producer_transaction::data() {
}

void slag::stream_producer_transaction::produce(size_t byte_count) {
}

void slag::stream_producer_transaction::resize(size_t size) {
}

void slag::stream_producer_transaction::set_state(stream_transaction_state state) {
    state_ = state;
}
