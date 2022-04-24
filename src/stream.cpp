#include "slag/stream.h"
#include "slag/util.h"

slag::stream::stream(size_t minimum_capacity)
    : buffer_{std::make_shared<stream_buffer>(minimum_capacity)}
    , producer_sequence_{0}
    , consumer_sequence_{0}
{
}

void slag::stream::add_producer(stream_producer& producer) {
    assert(std::find(producers_.begin(), producers_.end(), &producer) == producers_.end());
    assert(producer.sequence() == producer_sequence_);

    producers_.push_back(&producer);
}

void slag::stream::remove_producer(stream_producer& producer) {
    if (auto it = std::find(producers_.begin(), producers_.end(), &producer); it != producers_.end()) {
        producers_.erase(it);
    }
}

void slag::stream::advance_producer_sequence(size_t byte_count) {
    if (byte_count) {
        producer_sequence_ += byte_count;
        for_each_safe(consumers_, [this](stream_consumer* consumer) {
            consumer->notify_consumable();
        });
    }
}
