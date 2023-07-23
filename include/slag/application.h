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
#include "../generated/record.h"

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

    template<typename T1, typename T2>
    class SpscConnection {
    public:
        SpscConnection(size_t capacity)
            : SpscConnection(capacity, capacity)
        {
        }

        SpscConnection(size_t capacity1, size_t capacity2)
            : queue1_{capacity1}
            , queue2_{capacity2}
        {
        }

        template<typename U>
        [[nodiscard]] SpscQueueProducer<U> make_producer() {
            if constexpr (std::is_same_v<U, T1>) {
                return SpscQueueProducer<T1>{queue1_};
            }
            if constexpr (std::is_same_v<U, T2>) {
                return SpscQueueProducer<T2>{queue2_};
            }

            throw std::runtime_error("Unexpected type");
        }

        template<typename U>
        [[nodiscard]] SpscQueueConsumer<U> make_consumer() {
            if constexpr (std::is_same_v<U, T1>) {
                return SpscQueueConsumer<T1>{queue1_};
            }
            if constexpr (std::is_same_v<U, T2>) {
                return SpscQueueConsumer<T2>{queue2_};
            }

            throw std::runtime_error("Unexpected type");
        }

    private:
        SpscQueue<T1> queue1_;
        SpscQueue<T2> queue2_;
    };

    using WorkerThreadConnection = SpscConnection<WorkerThreadRequest, WorkerThreadReply>;

    // explicit start/stop messages to parallelize initialization/destruction
    class WorkerThread {
    public:
        explicit WorkerThread(const WorkerThreadConfig& config);
        ~WorkerThread();

        [[nodiscard]] std::future<void> run();

    private:
        friend class ControllerThread;

        using ControllerConnection = SpscConnection<WorkerThreadRequest, WorkerThreadReply>;

        [[nodiscard]] ControllerConnection& controller_connection();

    private:
        const WorkerThreadConfig&              config_;
        std::thread                            thread_;
        std::promise<void>                     completion_;
        ControllerConnection                   controller_connection_;
        SpscQueueConsumer<WorkerThreadRequest> controller_connection_consumer_;
        SpscQueueProducer<WorkerThreadReply>   controller_connection_producer_;
    };

    class ControllerThread {
    public:
        ControllerThread(const ControllerThreadConfig& config);
        ~ControllerThread();

        [[nodiscard]] std::future<void> run();

        void add_worker_thread(WorkerThread& worker_thread);

    private:
        class WorkerThreadContext {
        public:
            explicit WorkerThreadContext(WorkerThreadConnection& connection);

            [[nodiscard]] WorkerThreadState desired_state() const;
            [[nodiscard]] WorkerThreadState effective_state() const;
            [[nodiscard]] uint64_t effective_epoch() const;

            void set_desired_state(WorkerThreadState state);

            void step(uint64_t epoch);

        private:
            [[nodiscard]] void handle_reply(uint64_t request_id, const TransitionReply& request);
            [[nodiscard]] void handle_reply(uint64_t request_id, const TickReply& request);

        private:
            SpscQueueProducer<WorkerThreadRequest> producer_;
            SpscQueueConsumer<WorkerThreadReply>   consumer_;

            WorkerThreadState                      desired_state_;
            WorkerThreadState                      effective_state_;
            uint64_t                               effective_epoch_;

            std::optional<uint64_t>                transition_request_id_;
            std::optional<uint64_t>                tick_request_id_;
        };

    private:
        const ControllerThreadConfig&    config_;
        std::thread                      thread_;
        std::promise<void>               completion_;
        std::vector<WorkerThreadContext> worker_threads_;
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
        std::vector<std::unique_ptr<WorkerThread>> worker_threads_;
        std::unique_ptr<ControllerThread>          controller_thread_;
    };

}
