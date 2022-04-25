#include "slag/stream.h"
#include "slag/util.h"
#include <type_traits>
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
    for ()
}

void slag::stream::add_producer(stream_producer& producer) {
}

void slag::stream::remove_producer(stream_producer& producer) {
}

void slag::stream::advance_producer_sequence(size_t byte_count) {
}

size_t slag::stream::active_producer_transaction_count() const {
}

void slag::stream::add_consumer(stream_consumer& consumer) {
}

void slag::stream::remove_consumer(stream_consumer& consumer) {
}

void slag::stream::update_consumer_sequence() {
}

size_t slag::stream::active_consumer_transaction_count() const {
}

void slag::stream::add_observer(stream_observer& observer) {
    stream_event event_mask = observer.event_mask;
    if (static_cast<bool>(event_mask & DATA_PRODUCED)) {
        producer_observers_.push_back(&observer);
    }
    if (static_cast<bool>(event_mask & DATA_CONSUMED)) {
        consumer_observers_.push_back(&observer);
    }
}

void slag::stream::remove_observer(stream_observer& observer) {
    stream_event event_mask = observer.event_mask;
    if (static_cast<bool>(event_mask & DATA_PRODUCED)) {
        bool removed = pop_and_swap(producer_observers_, &observer);
        assert(removed);
    }
    if (static_cast<bool>(event_mask & DATA_CONSUMED)) {
        bool removed = pop_and_swap(consumer_observers_, &observer);
        assert(removed);
    }
}

void slag::stream::notify_observers(stream_event event) {
    switch (event) {
        case stream_event::DATA_PRODUCED: {
            stable_for_each(producer_observers_, [](stream_observer* observer) {
                observer->on_stream_data_produced();
            });
            break;
        }
        case stream_event::DATA_CONSUMED: {
            stable_for_each(consumer_observers_, [](stream_observer* observer) {
                observer->on_stream_data_consumed();
            });
            break;
        }
        default: {
            assert(false); // should be a discrete event, not an event mask
            break;
        }
    }
}

slag::stream_observer::stream_observer(stream& s, stream_event event_mask)
    : stream_{&s}
    , event_mask_{event_mask}
{
    if (stream_) {
        stream_->add_observer(*this);
    }
}

slag::stream_observer::~stream_observer() {
    if (stream_) {
        stream_->remove_observer(*this);
    }
}

slag::stream_event slag::stream_observer::event_mask() const {
    return event_mask_;
}

void slag::stream_observer::on_stream_data_produced() {
    assert(false); // override if you care about this event...
}

void slag::stream_observer::on_stream_data_consumed() {
    assert(false); // override if you care about this event...
}

void slag::stream_observer::abandon() {
    stream_ = nullptr;
}
