#include "slag/system/system_service.h"

namespace slag {

    static constexpr PENDING_DELETION_WATERMARK = 1000;

    SystemService::SystemService(ServiceRegistry& service_registry)
        : SystemServiceInterface{service_registry}
        , reactor_{pending_submissions()}
        , reaper_{*this}
    {
        reactor_.set_interrupt_handler(*this);
    }

    void SystemService::start_service() {
        ServiceInterface<ServiceType::SYSTEM>::start_service();

        get_scheduler_service(service_registry()).schedule_task(reaper_);
    }

    void SystemService::stop_service() {
        // Ensure that all active operations have been canceled.
        while (metrics().operations.total_active_count() > 0) {
            bool non_blocking = false;
            poll(non_blocking);
        }

        // Ensure that all operations have been deleted to make the allocator(s) happy.
        reap_operations();
        assert(!pending_deletions().is_ready());

        ServiceInterface<ServiceType::SYSTEM>::stop_service();
    }

    bool SystemService::poll(bool non_blocking) {
        // This is needed to ~bound the number of pending deletions. We can't rely
        // on the operation reaper task  keeping up, since it depends on the task load
        // being sufficiently low to run.
        while (pending_deletions().ready_count() > PENDING_DELETION_WATERMARK) {
            bool incremental = true;
            reap_operations(incremental);
        }

        return reactor_.poll(non_blocking);
    }

    void SystemService::handle_operation_started(OperationBase& operation_base) {
        ServiceInterface<ServiceType::SYSTEM>::handle_operation_started(operation_base);

        // Immediately forward operations to the reactor for submission. Another
        // implementation could be more clever and rate limit / audit these.
        pending_submissions().insert<PollableType::WRITABLE>(operation_base);
    }

    void SystemService::handle_operation_abandoned(OperationBase& operation_base) {
        ServiceInterface<ServiceType::SYSTEM>::handle_operation_abandoned(operation_base);
    }

    void SystemService::handle_operation_daemonized(OperationBase& operation_base) {
        ServiceInterface<ServiceType::SYSTEM>::handle_operation_daemonized(operation_base);
    }

    SystemService::OperationReaper::OperationReaper(SystemService& system_service)
        : ProtoTask(TaskPriority::IDLE)
        , system_service_(system_service)
    {
    }

    void SystemService::OperationReaper::run() {
        SLAG_PT_BEGIN();

        while (true) {
            SLAG_PT_WAIT_READABLE(system_service_.pending_deletions());

            bool incremental = true;
            size_t count = system_service_.reap_operations(incremental);
            (void)count;
        }

        SLAG_PT_END();
    }

}
