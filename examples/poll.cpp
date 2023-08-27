#include <iostream>
#include <span>
#include <bitset>

#include "slag/intrusive_queue.h"
#include "slag/slag.h"

#include <sys/mman.h>

using namespace slag::postal;

int main(int, char**) {
    Empire::Config empire_config;
    empire_config.index = 0;

    Nation::Config nation_config;
    nation_config.index                 = 0;
    nation_config.buffer_count          = 1024;
    nation_config.region_count          = 1;
    nation_config.parcel_queue_capacity = 512;

    Region::Config region_config;
    region_config.index        = 0;
    region_config.buffer_range = std::make_pair(1, 1024);

    Empire empire_{empire_config};
    Nation nation_{nation_config};
    Region region_{region_config};

    Executor executor;
    while (executor.is_runnable()) {
        executor.run();
    }

    CohortBufferAllocator allocator{region_};

    char greeting[] = "Hello, World!\0";

    BufferWriter buffer_writer{allocator};
    buffer_writer.write(std::as_bytes(std::span{greeting}));
    buffer_writer.write(std::as_bytes(std::span{greeting}));
    BufferHandle handle = buffer_writer.publish();

    BufferReader reader{std::move(handle)};

    size_t size = reader.size();
    std::cout << (const char*)reader.read(size).data() << std::endl;
    std::cout << reader.tell() << std::endl;
    std::cout << (const char*)reader.read(size).data() << std::endl;
    std::cout << reader.tell() << std::endl;

    return 0;
}
