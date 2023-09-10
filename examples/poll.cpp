#include <iostream>
#include <span>
#include <bitset>

#include "slag/intrusive_queue.h"
#include "slag/slag.h"

using namespace slag::postal;

class TestProtoThread : public slag::postal::ProtoTask {
public:
    void run() override final {
        SLAG_PT_BEGIN();

        {
            open_operation_ = make_open_operation("/tmp/foo", O_RDWR|O_CREAT);
        }

        SLAG_PT_WAIT_COMPLETE(*open_operation_);
        if (auto&& result = open_operation_->result(); result) {
            file_ = std::move(result.value());
            open_operation_.reset();
        }
        else {
            set_failure();
            return;
        }

        for (i_ = 0; i_ < 10; ++i_) {
            {
                const char message[] = "Hello, World!\n";

                BufferWriter writer;
                writer.write(std::as_bytes(std::span{message}));
                write_operation_ = make_write_operation(
                    file_, uint64_t{0},
                    writer.publish(), size_t{0}
                );
            }

            SLAG_PT_WAIT_COMPLETE(*write_operation_);
            if (auto&& result = write_operation_->result(); result) {
                std::cout << "Wrote " << result.value() << " bytes" << std::endl;
                write_operation_.reset();
            }
            else {
                set_failure();
                return;
            }
        }

        {
            std::cout << "One sec" << std::endl;
            timer_operation_ = make_timer_operation(std::chrono::seconds(1));

            SLAG_PT_WAIT_COMPLETE(*timer_operation_);
            if (auto&& result = timer_operation_->result(); result) {
                timer_operation_.reset();
            }
            else {
                set_failure();
                return;
            }
        }

        {
            socket_operation_ = make_socket_operation(AF_INET, SOCK_STREAM);

            SLAG_PT_WAIT_COMPLETE(*socket_operation_);
            if (auto&& result = socket_operation_->result(); result) {
                socket_ = std::move(result.value());
                socket_operation_.reset();
            }
            else {
                set_failure();
                return;
            }
        }

        SLAG_PT_END();
    }

private:
    FileHandle            file_;
    FileHandle            socket_;
    OpenOperationHandle   open_operation_;
    WriteOperationHandle  write_operation_;
    TimerOperationHandle  timer_operation_;
    SocketOperationHandle socket_operation_;
    int                   i_ = 0;
};

class MockTask : public Task {
public:
    MockTask()
        : state_{0}
        , open_operation_{make_open_operation("/tmp/foo", O_RDWR|O_CREAT)}
    {
    }

    Event& runnable_event() override final {
        switch (state_) {
            case 0: {
                return open_operation_->complete_event();
            }
            case 1: {
                return write_operation_->complete_event();
            }
        }

        abort();
    }

    void run() override final {
        switch (state_++) {
            case 0: {
                if (auto&& result = open_operation_->result()) {
                    std::cout << "open success" << std::endl;

                    file_ = std::move(result.value());
                    open_operation_.reset();

                    const char message[] = "Hello, World!\n";

                    BufferWriter writer;
                    writer.write(std::as_bytes(std::span{message}));

                    write_operation_ = make_write_operation(
                        file_, uint64_t{0},
                        writer.publish(), size_t{0}
                    );
                }
                else {
                    std::cout << "open failure" << std::endl;
                    set_failure();
                }
                break;
            }
            case 1: {
                auto&& result = write_operation_->result();
                if (result) {
                    std::cout << "write success - " << result.value() << " bytes written" << std::endl;
                    set_success();
                }
                else {
                    std::cout << "write failure" << std::endl;
                    set_failure();
                }
            }
        }
    }

private:
    int                  state_;
    FileHandle           file_;
    OpenOperationHandle  open_operation_;
    WriteOperationHandle write_operation_;
};

// This should be relatively pure and just call into the various
// event loop phases.
class EventLoop : public Task {
public:
    EventLoop()
        : reactor_{region().reactor()}
    {
        runnable_event_.set();
        executor_.insert(task_.emplace());
    }

    Event& runnable_event() override final {
        return runnable_event_;
    }

    void run() override final {
        if (executor_.is_runnable()) {
            executor_.run();
        }

        reactor_.poll();

        if (task_->is_complete()) {
            set_success(task_->is_success()) ;
            task_.reset();

            shutdown();
        }
    }

    void shutdown() {
        // Destroying this should cause operations to be canceled and file handles to be dropped.
        task_.reset();

        // Drive the reactor until all operations have completed.
        while (!reactor_.is_quiescent()) {
            reactor_.poll();
        }
    }

private:
    using InitTask = TestProtoThread;

    Reactor&                reactor_;
    Event                   runnable_event_;
    Executor                executor_;
    std::optional<InitTask> task_;
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
