#include <iostream>
#include <span>
#include <bitset>

#include "slag/intrusive_queue.h"
#include "slag/slag.h"

using namespace slag::postal;

// This should be relatively pure and just call into the various
// event loop phases.
class EventLoop : public Task {
public:
    EventLoop() {
        runnable_event_.set();
    }

    Event& runnable_event() override final {
        return runnable_event_;
    }

    void run() override final {
        set_success();
    }

private:
    Event runnable_event_;
};

using WorkerThread = Thread<EventLoop>;

int main(int, char**) {
    Empire::Config empire_config;
    empire_config.index = 0;
    Empire empire_{empire_config};

    Nation::Config nation_config;
    nation_config.index                 = 0;
    nation_config.buffer_count          = 16 * 1024 + 1;
    nation_config.region_count          = 4;
    nation_config.parcel_queue_capacity = 512;
    Nation nation_{nation_config};

    // Start worker threads.
    std::vector<std::unique_ptr<WorkerThread>> threads;
    threads.reserve(nation_config.region_count);
    for (size_t region_index = 0; region_index < nation_config.region_count; ++region_index) {
        size_t region_buffer_count = nation_config.buffer_count / nation_config.region_count;
        size_t region_buffer_range_beg = (region_index * region_buffer_count) + 1;
        size_t region_buffer_range_end = region_buffer_range_beg + region_buffer_count;

        Region::Config region_config;
        region_config.index = region_index;
        region_config.buffer_range = std::make_pair(region_buffer_range_beg, region_buffer_range_end);

        threads.push_back(
            std::make_unique<WorkerThread>(region_config)
        );
    }

    // Wait for threads to complete.
    for (auto&& thread: threads) {
        std::future<void> complete_future = thread->get_future();
        complete_future.get();
    }

    return 0;
}
