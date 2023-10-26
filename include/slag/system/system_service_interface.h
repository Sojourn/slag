#pragma once

#include <numeric>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "slag/core/service.h"
#include "slag/core/service_interface.h"
#include "slag/system/file_table.h"
#include "slag/system/file_handle.h"
#include "slag/system/operation.h"
#include "slag/system/operation_handle.h"
#include "slag/system/operation_allocator.h"
#include "slag/scheduling/scheduler_service_interface.h"

namespace slag {

    class ServiceRegistry;

    template<>
    class ServiceInterface<ServiceType::SYSTEM> : public Service {
    public:
        explicit ServiceInterface(ServiceRegistry& service_registry)
            : Service(ServiceType::SYSTEM, service_registry)
        {
        }

        void start_service() override {
            assert(metrics_.operations.total_active_count() == 0);
            assert(!pending_submissions_.is_ready());
            assert(!pending_deletions_.is_ready());
        }

        void stop_service() override {
            assert(metrics_.operations.total_active_count() == 0);
            assert(!pending_submissions_.is_ready());
            assert(!pending_deletions_.is_ready());
        }

        virtual bool poll(bool non_blocking) = 0;

    public:
        struct OperationMetrics {
            std::array<size_t, OPERATION_TYPE_COUNT> active_counts;
            std::array<size_t, OPERATION_TYPE_COUNT> daemon_counts;

            size_t total_active_count() const {
                return std::accumulate(active_counts.begin(), active_counts.end(), size_t{0});
            }

            size_t total_daemon_count() const {
                return std::accumulate(daemon_counts.begin(), daemon_counts.end(), size_t{0});
            }
        };

        struct Metrics {
            OperationMetrics operations;
        };

        const Metrics& metrics() const {
            return metrics_;
        }

        FileTable& file_table() {
            return file_table_;
        }

    public:
        template<OperationType operation_type, typename... Args>
        OperationHandle<operation_type> start_operation(Args&&... args) {
            Operation<operation_type>& operation = operation_allocator_.allocate<operation_type>(
                *this, std::forward<Args>(args)...
            );

            // Compound operations may be Task subclasses so they can run bits of logic
            // between child operation completions. Do this here where we have type information.
            if constexpr (std::is_base_of_v<Task, Operation<operation_type>>) {
                get_scheduler_service(service_registry()).schedule_task(operation);
            }

            handle_operation_started(static_cast<OperationBase&>(operation));

            return OperationHandle<operation_type>(operation);
        }

    protected:
        Selector& pending_submissions() {
            return pending_submissions_;
        }

        // Could make this a level-triggered queue instead.
        Selector& pending_deletions() {
            return pending_deletions_;
        }

    private:
        friend class OperationBase;

        // Cull a limited number of operations that are pending deletion.
        // Returns how many were culled.
        size_t reap_operations() {
            std::array<Event*, 32> events;

            size_t count = pending_deletions_.select(events);
            for (size_t i = 0; i < count; ++i) {
                visit([&](auto&& operation) {
                    operation_allocator_.deallocate(operation);
                }, *reinterpret_cast<OperationBase*>(events[i]->user_data()));
            }

            return count;
        }

        virtual void handle_operation_started(OperationBase& operation_base) {
            increment_operation_count(operation_base);
        }

        virtual void handle_operation_abandoned(OperationBase& operation_base) {
            visit([&](auto&& operation) {
                Event& event = operation.complete_event();

                // Adopt the operation so that we can reap it after it completes.
                if (event.is_linked()) {
                    event.unlink();
                }

                pending_deletions_.insert(event, &operation_base);
            }, operation_base);
        }

        virtual void handle_operation_daemonized(OperationBase& operation_base) {
            size_t index = to_index(operation_base.type());

            metrics_.operations.daemon_counts[index] += 1;
        }

        void increment_operation_count(const OperationBase& operation_base) {
            size_t index = to_index(operation_base.type());

            metrics_.operations.active_counts[index] += 1;
            if (operation_base.is_daemonized()) {
                assert(false); // Not fatal, but metrics will probably be off.
            }
        }

        void decrement_operation_count(const OperationBase& operation_base) {
            size_t index = to_index(operation_base.type());

            metrics_.operations.active_counts[index] -= 1;
            if (operation_base.is_daemonized()) {
                metrics_.operations.daemon_counts[index] -= 1;
            }
        }

    private:
        Metrics            metrics_;
        FileTable          file_table_;
        Selector           pending_submissions_;
        Selector           pending_deletions_;
        OperationAllocator operation_allocator_;
    };

}
