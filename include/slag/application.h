#pragma once

#include <string>
#include <future>
#include <optional>
#include <thread>
#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/spsc_queue.h"

namespace slag {

    class Application;
    class ControllerThread;
    class WorkerThread;

    struct ControllerThreadConfig {
        std::optional<int> cpu_affinity;
    };

    struct WorkerThreadConfig {
        std::optional<int> cpu_affinity;
        size_t             request_queue_capacity = 512;
        size_t             reply_queue_capacity   = 512;
    };

    struct ApplicationConfig {
        ControllerThreadConfig          controller_thread;
        std::vector<WorkerThreadConfig> worker_threads;
    };

    class ControllerThread {
    public:
        ControllerThread(const ControllerThreadConfig& config);
        ~ControllerThread();

        [[nodiscard]] std::future<void> run();

        // void set_controller_connection(SpscQueue<WorkerRequest>& request_queue, SpscQueue<WorkerReply>& reply_queue);

    private:
        const ControllerThreadConfig& config_;
        std::thread                   thread_;
        std::promise<void>            completion_;
    };

    // explicit start/stop messages to parallelize initialization/destruction
    class WorkerThread {
    public:
        explicit WorkerThread(const WorkerThreadConfig& config);
        ~WorkerThread();

        [[nodiscard]] std::future<void> run();

        // void add_worker_connection(SpscQueue<WorkerRequest>& request_queue, SpscQueue<WorkerReply>& reply_queue);

    private:
        const WorkerThreadConfig& config_;
        std::thread               thread_;
        std::promise<void>        completion_;
    };

    class Application {
        Application(Application&&) = delete;
        Application(const Application&) = delete;
        Application& operator=(Application&&) = delete;
        Application& operator=(const Application&) = delete;

    public:
        using Config = ApplicationConfig;

        explicit Application(const Config& config);
        ~Application();

        [[nodiscard]] int run();

    private:
        const Config&                              config_;
        std::unique_ptr<ControllerThread>          controller_thread_;
        std::vector<std::unique_ptr<WorkerThread>> worker_threads_;
    };

}
