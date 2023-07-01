#include "catch.hpp"
#include "slag/slag.h"
#include <cstring>

using namespace slag;

TEST_CASE("SpscQueue") {
    // ensure the producer/consumer members fit in a single cache line
    static_assert(sizeof(SpscQueueProducer<int>) == 64);
    static_assert(sizeof(SpscQueueConsumer<int>) == 64);

    // ensure the producer/consumer are properly aligned
    static_assert(alignof(SpscQueueProducer<int>) == 64);
    static_assert(alignof(SpscQueueConsumer<int>) == 64);

    SECTION("Single reads and writes") {
        SpscQueue<int> queue{13};
        CHECK(queue.capacity() == 16);

        SpscQueueProducer<int> producer{queue};
        SpscQueueConsumer<int> consumer{queue};

        for (int i = 0; i < 1024; ++i) {
            int producer_value = i;
            bool producer_success = producer.produce(producer_value);
            CHECK(producer_success);

            int consumer_value = -1;
            bool consumer_success = consumer.consume(consumer_value);
            CHECK(consumer_success);
            CHECK(consumer_value == producer_value);
        }
    }

    SECTION("Batched reads and writes") {
        SpscQueue<int> queue{16};
        SpscQueueProducer<int> producer{queue};
        SpscQueueConsumer<int> consumer{queue};

        constexpr int stride = 4;
        for (int i = 0; i < 1024; i += stride) {
            int producer_values[stride];
            for (int j = 0; j < stride; ++j) {
                producer_values[j] = i + j;
            }
            size_t produced_value_count = producer.produce(std::span{producer_values});
            CHECK(produced_value_count == size_t{stride});

            int consumed_values[stride];
            memset(consumed_values, 0, sizeof(consumed_values));
            size_t consumed_value_count = consumer.consume(std::span{consumed_values});
            CHECK(consumed_value_count == size_t{stride});

            for (int j = 0; j < stride; ++j) {
                CHECK(consumed_values[j] == i + j);
            }
        }
    }
}
