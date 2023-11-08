#pragma once

#include "slag/core/service.h"
#include "slag/core/service_interface.h"
#include "slag/memory/buffer_ledger.h"

namespace slag {

    class ServiceRegistry;

    template<>
    class ServiceInterface<ServiceType::MEMORY> : public Service {
    public:
        ServiceInterface(ServiceRegistry& service_registry, BufferLedger& ledger)
            : Service(ServiceType::MEMORY, service_registry)
            , ledger_{ledger}
        {
        }

        void start_service() override {
        }

        void stop_service() override {
        }

    private:
        friend class BufferHandle;

        void increment_reference_count(BufferDescriptor descriptor) {
            LocalReferenceCount& reference_count = local_reference_counts_[descriptor.index];
        }

        void decrement_reference_count(BufferDescriptor descriptor) {
            LocalReferenceCount& reference_count = local_reference_counts_[descriptor.index];
        }

    private:
        friend class BufferAllocator;

        BufferDescriptor allocate_descriptor() {
            if (descriptor_pool_.empty()) {
                throw std::runtime_error("Failed to allocate buffer descriptor");
            }

            uint32_t index = descriptor_pool_.back();
            descriptor_pool_.pop_back();

            return {
                .index = index,
            };
        }

        void deallocate_descriptor(BufferDescriptor descriptor) {
            NationalEntry& national_entry = get_national_entry(descriptor);
            national_entry.allocator->deallocate_buffer(descriptor);
            descriptor_pool_.push_back(descriptor.index);
        }

        BufferSegment& allocate_segment() {
            BufferSegment& segment = segment_allocator_.allocate();
            memset(&segment, 0, sizeof(segment));
            return segment;
        }

        void deallocate_segment(BufferSegment& segment) {
            segment_allocator_.deallocate(segment);
        }

    private:
        using LocalReferenceCount = int8_t;

        BufferLedger&                    ledger_;
        PoolAllocator<BufferSegment>     segment_pool_;
        std::vector<uint32_t>            descriptor_pool_;
        std::vector<LocalReferenceCount> local_reference_counts_;
    };

}
