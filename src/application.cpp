#include "slag/application.h"
#include "slag/logging.h"
#include "slag/util.h"
#include <cassert>

namespace slag {

    ControllerThread::ControllerThread(const ControllerThreadConfig& config)
        : config_{config}
    {
    }

    ControllerThread::~ControllerThread() {
        thread_.join();
    }

    std::future<void> ControllerThread::run() {
        std::future<void> result = completion_.get_future();

        thread_ = std::thread([this]() {
            try {
                if (config_.cpu_affinity) {
                    set_cpu_affinity(*config_.cpu_affinity);
                }

                // ...

                completion_.set_value();
            }
            catch (...) {
                completion_.set_exception(std::current_exception());
                return;
            }
        });

        return result;
    }

    void ControllerThread::add_worker_thread(WorkerThread& worker_thread) {
        worker_threads_.push_back(
            WorkerThreadContext {
                worker_thread.controller_connection()
            }
        );
    }

    ControllerThread::WorkerThreadContext::WorkerThreadContext(WorkerThreadConnection& connection)
        : producer_{connection.make_producer<WorkerThreadRequest>()}
        , consumer_{connection.make_consumer<WorkerThreadReply>()}
    {
    }

    WorkerThread::WorkerThread(const WorkerThreadConfig& config)
        : config_{config}
        , controller_connection_{config_.request_queue_capacity, config_.reply_queue_capacity}
        , controller_connection_consumer_{controller_connection_.make_consumer<WorkerThreadRequest>()}
        , controller_connection_producer_{controller_connection_.make_producer<WorkerThreadReply>()}
    {
    }

    WorkerThread::~WorkerThread() {
        thread_.join();
    }

    std::future<void> WorkerThread::run() {
        std::future<void> result = completion_.get_future();

        thread_ = std::thread([this]() {
            try {
                if (config_.cpu_affinity) {
                    set_cpu_affinity(*config_.cpu_affinity);
                }

                // ...

                completion_.set_value();
            }
            catch (...) {
                completion_.set_exception(std::current_exception());
                return;
            }
        });

        return result;
    }

    auto WorkerThread::controller_connection() -> ControllerConnection& {
        return controller_connection_;
    }

    Application::Application(const Config& config)
        : config_{config}
    {
        for (const WorkerThreadConfig& worker_thread_config: config_.worker_threads) {
            worker_threads_.push_back(
                std::make_unique<WorkerThread>(worker_thread_config)
            );
        }

        controller_thread_ = std::make_unique<ControllerThread>(
            config_.controller_thread
        );
        for (auto&& worker_thread: worker_threads_) {
            controller_thread_->add_worker_thread(*worker_thread);
        }
    }

    Application::~Application() {
    }

    int Application::run() {
        std::vector<std::future<void>> completion_futures;

        // run threads, starting with the controller
        completion_futures.reserve(worker_threads_.size() + 1);
        completion_futures.push_back(controller_thread_->run());
        for (auto&& worker_thread: worker_threads_) {
            completion_futures.push_back(worker_thread->run());
        }

        // wait for threads to complete in the opposite order they were started
        bool success = true;
        for (auto it = completion_futures.rbegin(); it != completion_futures.rend(); ++it) {
            try {
                it->get();
            }
            catch (const std::exception& ex) {
                success = false;
                fatal("Thread terminated with an exception: {}", ex.what());
            }
        }

        return success ? 0 : -1;
    }

}
