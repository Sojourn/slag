#include <iostream>
#include <thread>
#include <cstdint>
#include <cstddef>
#include "slag/spsc_queue.h"

#include <sched.h>

using namespace slag;

void set_affinity(int core) {
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(core, &set);

    if (::sched_setaffinity(0, sizeof(set), &set) < 0) {
        abort();
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    constexpr uint64_t capacity            = 16 * 1024;
    constexpr uint64_t iterations          = 100000000;
    constexpr uint64_t producer_batch_size = 512;
    constexpr uint64_t consumer_batch_size = 512;

    SpscQueue<uint64_t> queue{capacity};

    std::thread producer_thread([&]() {
        set_affinity(1);

        SpscQueueProducer<uint64_t> producer{queue};

        uint64_t sum = 0;
        for (uint64_t i = 0; i < iterations; ) {
            uint64_t batch[producer_batch_size];
            for (uint64_t j = 0; j < producer_batch_size; ++j) {
                batch[j] = i + j;
            }

            if (producer.insert(std::span<const uint64_t>{batch})) {
                i += producer_batch_size;

                for (uint64_t item: batch) {
                    sum += item;
                }
            }
        }

        std::cout << "Producer: " << sum << std::endl;
    });

    std::thread consumer_thread([&]() {
        set_affinity(3);

        SpscQueueConsumer<uint64_t> consumer{queue};

        uint64_t sum = 0;
        for (uint64_t i = 0; i < iterations; ) {
            uint64_t* items[consumer_batch_size];

            size_t count = consumer.poll(items);
            for (size_t j = 0; j < count; ++j) {
                sum += *items[j];
            }

            i += count;
            consumer.remove(count);
        }

        std::cout << "Consumer: " << sum << std::endl;
    });

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
