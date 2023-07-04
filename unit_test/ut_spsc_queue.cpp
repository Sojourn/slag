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
            bool producer_success = producer.insert(producer_value);
            CHECK(producer_success);
            producer.flush();

            int* consumer_value;
            size_t count = consumer.poll(std::span{&consumer_value, 1});
            CHECK(count == 1);
            CHECK(*consumer_value == producer_value);

            consumer.remove(1);
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
            bool producer_success = producer.insert(std::span<const int>{producer_values});
            CHECK(producer_success);
            producer.flush();

            int* consumed_values[stride];
            size_t consumed_value_count = consumer.poll(std::span{consumed_values, stride});
            CHECK(consumed_value_count == size_t{stride});

            for (int j = 0; j < stride; ++j) {
                CHECK(*consumed_values[j] == i + j);
            }

            consumer.remove(consumed_value_count);
        }
    }
}
