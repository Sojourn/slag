#include <iostream>
#include <span>
#include <bitset>

#include "slag/intrusive_queue.h"
#include "slag/slag.h"

using namespace slag::postal;

template<typename TaskImpl>
class SupervisedTaskGroup : public TaskGroup<TaskImpl> {
public:
    template<typename... Args>
    SupervisedTaskGroup(Args&&... args)
        : TaskGroup<TaskImpl>(std::forward<Args>(args)...)
        , failure_count_{0}
        , max_failure_count_{std::numeric_limits<size_t>::max()}
    {
    }

    void reset_failure_count() {
        failure_count_ = 0;
    }

    void set_max_failure_count(size_t max_failure_count) {
        max_failure_count_ = max_failure_count;
    }

private:
    // Reap a task that was destroyed outside of our destructor.
    void reap(TaskImpl& task) override final {
        if (task.is_failure()) {
            failure_count_ += 1;
            if (failure_count_ == max_failure_count_) {
                Task::set_failure(); // Suicide the task group.
                return;
            }
        }
    }

private:
    size_t failure_count_;
    size_t max_failure_count_;
};

// BufferAllocator

// BufferWriter(BufferAllocator)
//  lazy allocation
// .write(span, offset?)
//  publish?commit/rollback/etc.?
//    BufferHandle is read-only once published

// BufferReader(BufferHandle)

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
    region_config.buffer_range = std::make_pair(0, 1024);

    Empire empire_{empire_config};
    Nation nation_{nation_config};
    Region region_{region_config};

    Executor executor;
    while (executor.is_runnable()) {
        executor.run();
    }

    return 0;
}
