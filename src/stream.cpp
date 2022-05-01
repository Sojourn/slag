#include "slag/stream.h"
#include "slag/util.h"
#include <type_traits>
#include <limits>
#include <cassert>

slag::stream_event slag::operator&(stream_event l, stream_event r) {
    return static_cast<stream_event>(
        static_cast std::underlying_type_t<stream_event>(l) & static_cast std::underlying_type_t<stream_event>(r);
    );
}

void slag::operator&=(stream_event& l, stream_event r) {
    l = l & r;
}

slag::stream_event slag::operator|(stream_event l, stream_event r) {
    return static_cast<stream_event>(
        static_cast std::underlying_type_t<stream_event>(l) | static_cast std::underlying_type_t<stream_event>(r);
    );
}

void slag::operator|=(stream_event& l, stream_event r) {
    l = l | r;
}

slag::stream_event slag::operator^(stream_event l, stream_event r) {
    return static_cast<stream_event>(
        static_cast std::underlying_type_t<stream_event>(l) ^ static_cast std::underlying_type_t<stream_event>(r);
    );
}

void slag::operator^=(stream_event& l, stream_event r) {
    l = l ^ r;
}

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
    for (auto&& observers: {&producer_observers_, &consumer_observers_}) {
        while (!observers->empty()) {
            auto&& observer = observers.front();
            observer->remove_hook(*this);
        }
    }
}

void slag::stream::add_observer(stream_observer& observer, stream_event event_mask) {
    if (observer.has_hook(*this)) {
        throw std::runtime_error("stream observer has already been added to stream");
    }

    if (static_cast<bool>(event_mask & DATA_PRODUCED)) {
        producer_observers_.push_back(&observer);
    }
    if (static_cast<bool>(event_mask & DATA_CONSUMED)) {
        consumer_observers_.push_back(&observer);
    }

    observer.add_hook(*this, event_mask);
}

void slag::stream::remove_observer(stream_observer& observer) {
    std::optional<stream_event> event_mask = observer.find_hook(*this);
    if (!event_mask) {
        assert(false); // post-condition holds (so fine in release), but still probably a bug
        return;
    }

    if (static_cast<bool>(event_mask & DATA_PRODUCED)) {
        bool removed = swap_and_pop(producer_observers_, &observer);
        assert(removed);
    }
    if (static_cast<bool>(event_mask & DATA_CONSUMED)) {
        bool removed = swap_and_pop(consumer_observers_, &observer);
        assert(removed);
    }

    observer.remove_hook(*this);
}

slag::stream_buffer& slag::stream::buffer() {
    return *buffer_;
}

std::shared_ptr<slag::stream_buffer> slag::stream::shared_buffer() {
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

    for (stream_consumer& consumer: consumers_) {
        if (stream_consumer_transaction* transaction = consumer.transaction()) {
            if (transaction->state() == stream_transaction_state::ACTIVE) {
                transaction->set_buffer(buffer_);
            }
        }
    }

    std::exchange(buffer_, new_buffer);
}

void slag::stream::advance_producer_sequence(size_t byte_count) {
    assert(byte_count <= producer_segment_size())
    producer_sequence_ += byte_count;
    notify_observers(stream_event::DATA_PRODUCED);
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
        notify_observers(stream_event::DATA_CONSUMED);
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

void slag::stream::notify_observers(stream_event event) {
    std::vector<stream_observer*>* observers = nullptr;
    switch (event) {
        case stream_event::DATA_PRODUCED: {
            observers = &producer_observers_;
            break;
        }
        case stream_event::DATA_CONSUMED: {
            observers = &consumer_observers_;
            break;
        }
        default: {
            assert(false); // should be a discrete event, not an event mask
            return;
        }
    }

    stable_for_each(*observers, [](stream_observer* observer) {
        observer->on_stream_event(event);
    });
}

slag::stream_producer::stream_producer(stream& stream) {
}

slag::stream_producer::~stream_producer() {
}

slag::stream_sequence slag::stream_producer::sequence() const {
}

bool slag::stream_producer::has_active_transaction() const {
}

slag::stream_producer_transaction slag::stream_producer::make_transaction() {
}

void slag::stream_producer::abandon() {
}

slag::stream_consumer::stream_consumer(stream& s)
    : stream_{&s}
{
}

slag::stream_consumer::~stream_consumer() {
    if (stream_) {
        stream_->remove_consumer(*this);
    }
}

slag::stream_sequence slag::stream_consumer::sequence() const {
    return sequence_;
}

bool slag::stream_consumer::has_active_transaction() const {
}

slag::stream_consumer_transaction slag::stream_consumer::make_transaction() {
}

void slag::stream_consumer::abandon() {
    stream_ = nullptr;
    if (transaction_) {
        transaction_->set_state(stream_transaction_state::ABORTED);
    }
}

void slag::stream_consumer::transaction_complete() {
    stream_consumer_transaction* transaction = std::exchange(transaction_, nullptr);
    switch (transaction->state()) {
        case stream_transaction_state::COMMITTED: {
            sequence_ = transaction->consumer_sequence();
            if (stream_) {
                stream_->update_consumer_sequence();
            }
            break;
        }
        default: {
            break;
        }
    }
}

slag::stream_observer::~stream_observer() {
    while (!hooks_.empty()) {
        auto&& [subject, _] = *hooks_.begin();
        subject->remove_observer(*this);
    }
}

bool slag::has_hook(const stream& subject) const {
    return hooks_.contains(&subject);
}

void slag::stream_observer::add_hook(stream& subject, stream_event event_mask) {
    auto&& [_, inserted] = hooks_.insert(std::make_pair(&subject, event_mask));
    assert(inserted);
}

void slag::stream_observer::remove_hook(stream& subject) {
    auto it = hooks_.find(&subject);
    assert(it != hooks_.end());
    hooks_.erase(it);
}
