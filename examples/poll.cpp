#include <iostream>
#include <span>
#include <bitset>

#include "slag/intrusive_queue.h"
#include "slag/slag.h"

using namespace slag::postal;

class TestProtoTask : public ProtoTask {
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

class Server;

class ServerConnection : public ProtoTask {
public:
    ServerConnection(Server& server, FileHandle socket)
        : server_{server}
        , socket_{std::move(socket)}
    {
    }

    void run() override final {
        SLAG_PT_BEGIN();

        slag::info("Writing MOTD to socket");
        {
            const char message[] = "MOTD\n";

            BufferWriter writer;
            writer.write(std::as_bytes(std::span{message}));
            write_operation_ = make_write_operation(
                socket_, uint64_t{0},
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

        SLAG_PT_END();
    }

private:
    Server&              server_;
    FileHandle           socket_;
    WriteOperationHandle write_operation_;
};

class Server : public ProtoTask {
public:
    explicit Server(const slag::Address& address)
        : address_{address}
    {
    }

    void run() override final {
        SLAG_PT_BEGIN();

        // Connections can inherit the executor driving us.
        region().current_executor().insert(connections_);

        // Setup the listening socket.
        {
            socket_operation_ = make_socket_operation(AF_INET, SOCK_STREAM, 0);
            SLAG_PT_WAIT_COMPLETE(*socket_operation_);

            auto&& result = socket_operation_->result();
            if (result.has_error()) {
                slag::error("[Server] failed to open socket - {}", to_string(result.error()));
                set_failure();
                return;
            }

            socket_ = std::move(result.value());
            socket_operation_.reset();

            if (::bind(socket_.file_descriptor(), &address_.addr(), address_.size()) < 0) {
                slag::error("[Server] failed to bind socket - {}", to_string(slag::make_system_error()));
                set_failure();
                return;
            }

            int backlog = 4096;
            if (::listen(socket_.file_descriptor(), backlog) < 0) {
                slag::error("[Server] failed to listen on socket - {}", to_string(slag::make_system_error()));
                set_failure();
                return;
            }

            slag::info("[Server] accepting connections on port:{}", ntohs(address_.addr_in().sin_port));
        }

        // Accept connections.
        {
            accept_operation_ = make_accept_operation(socket_);
            while (true) {
                SLAG_PT_WAIT_READABLE(*accept_operation_);

                auto&& results = accept_operation_->results();
                while (auto&& result = results.pop_front()) {
                    if (result->has_error()) {
                        slag::error("[Server] failed to accept socket - {}", to_string(result->error()));
                        set_failure();
                        return;
                    }

                    connections_.spawn(*this, std::move(result->value()));
                }
            }
        }

        SLAG_PT_END();
    }

private:
    slag::Address               address_;
    FileHandle                  socket_;
    SocketOperationHandle       socket_operation_;
    AcceptOperationHandle       accept_operation_;
    TaskGroup<ServerConnection> connections_;
};

template<typename InitTask>
class EventLoop : public Task {
public:
    template<typename... Args>
    EventLoop(Args&&... args)
        : reactor_{region().reactor()}
    {
        runnable_event_.set();

        executor_.insert(
            task_.emplace(std::forward<Args>(args)...)
        );
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
    Reactor&                reactor_;
    Event                   runnable_event_;
    Executor                executor_;
    std::optional<InitTask> task_;
};

using WorkerThread = Thread<
    EventLoop<Server>
>;

slag::Address make_address(int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<short>(port));

    return slag::Address{addr};
}

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
            std::make_unique<WorkerThread>(
                region_config,
                make_address(10000 + static_cast<int>(region_index))
            )
        );
    }

    // Wait for threads to complete.
    for (auto&& thread: threads) {
        std::future<void> complete_future = thread->get_future();
        complete_future.get();
    }

    return 0;
}
