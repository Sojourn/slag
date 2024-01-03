#include "thread.h"
#include "application.h"

namespace slag {

    Thread::Thread(Application& application, std::unique_ptr<Task> task)
        : application_(application)
        , components_(nullptr)
        , thread_()
        , index_(application.attach(*this))
        , task_(std::move(task))
    {
    }

    Thread::~Thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    ThreadIndex Thread::index() const {
        return index_;
    }

    ResourceTables& Thread::resource_tables() {
        assert(components_);

        return *components_->resource_tables;
    }

    EventLoop& Thread::event_loop() {
        assert(components_);

        return *components_->event_loop;
    }

    void Thread::start() {
        assert(!thread_.joinable());

        thread_ = std::thread([this]() {
            try {
                run();
            }
            catch (const std::exception& exception) {
                // TODO: log
                (void)exception;
            }

            application_.handle_stopped(*this);
        });
    }

    void Thread::run() {
        Context context(application_, *this);

        Components components;
        components_ = &components;
        {
            ResourceTables resource_tables;
            components.resource_tables = &resource_tables;

            EventLoop event_loop;
            components.event_loop = &event_loop;

            event_loop.schedule(*task_);
            event_loop.loop();
        }
        components_ = nullptr;
    }

}
