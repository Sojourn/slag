#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/spsc_queue.h"
#include "slag/postal/types.h"
#include "slag/postal/config.h"
#include "slag/postal/season.h"
#include "slag/postal/census.h"
#include "slag/postal/buffer.h"
#include "slag/postal/parcel.h"
#include "slag/postal/post_office.h"
#include "slag/postal/executor.h"

namespace slag::postal {

    class Empire;
    class Nation;
    class Region;

    Empire& empire();
    Nation& nation();
    Region& region();

    class Domain {
        Domain(Domain&&) = delete;
        Domain(const Domain&) = delete;
        Domain& operator=(Domain&&) = delete;
        Domain& operator=(const Domain&) = delete;

    protected:
        Domain(DomainType type, size_t index);
        ~Domain();

    public:
        DomainType type() const;
        size_t index() const;

    private:
        DomainType type_;
        size_t     index_;
    };

    class Empire : public Domain {
    public:
        using Config = DomainConfig<DomainType::EMPIRE>;
        using Census = DomainCensus<DomainType::EMPIRE>;

        explicit Empire(const Config& config);

        const Config& config() const;

    private:
        Config config_;
    };

    class Nation : public Domain {
    public:
        using Config = DomainConfig<DomainType::NATION>;
        using Census = DomainCensus<DomainType::NATION>;

        explicit Nation(const Config& config);

        const Config& config() const;

    private:
        friend class Region;

        ParcelQueue& parcel_queue(PostArea to, PostArea from);

        void attach(Region& region);
        void detach(Region& region);

    private:
        Empire&                                   empire_;
        Config                                    config_;
        std::vector<Region*>                      regions_; // atomic/locking?
        std::vector<std::unique_ptr<ParcelQueue>> parcel_queues_;
    };

    // allocate buffers out of the sorted stack of recycled buffers; they
    // still have some temporal locality since they were recently touched/zeroed
    //
    // TODO: prefetch the next buffer entry on allocation
    //
    class Region : public Domain {
    public:
        using Config = DomainConfig<DomainType::REGION>;
        using Census = DomainCensus<DomainType::REGION>;

        explicit Region(const Config& config);
        ~Region();

        const Config& config() const;
        const Season& season() const;
        const Census& census() const;
        const Census& census(Season season) const;

        PostArea post_area() const;
        PostOffice& post_office();
        Executor& current_executor();

    public:
        // Return a cursor that always points at the Census for the current Season.
        Census** census_cursor();

        void enter_season(Season season);
        void leave_season(Season season);

    private:
        std::span<ParcelQueueConsumer> imports();
        std::span<ParcelQueueProducer> exports();

    private:
        PostArea make_post_area(size_t region_index) const;
        void make_history();

        void attach_parcel_queues();
        void detach_parcel_queues();
        void survey_parcel_queues();

    private:
        friend class Executor;

        // Push and pop the currently running executor.
        void enter_executor(Executor& executor);
        void leave_executor(Executor& executor);

    private:
        Nation&                          nation_;
        Config                           config_;
        Season                           season_;
        Census*                          census_cursor_;
        std::array<Census, SEASON_COUNT> history_;

        PostOffice                       post_office_;
        std::vector<ParcelQueueConsumer> imports_;
        std::vector<ParcelQueueProducer> exports_;

        std::vector<Executor*>           executor_stack_;
    };

}
