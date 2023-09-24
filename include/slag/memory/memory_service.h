#pragma once

#include <sys/mman.h>
#include "slag/util.h"
#include "slag/layer.h"
#include "slag/logging.h"
#include "slag/postal/service.h"
#include "slag/system/operation_factory.h"

namespace slag {

    // TODO: inherit a BlockAllocator interface and attach ourselves to the region.
    //
    template<typename Stack>
    class MemoryService
        : public Service
        , public Layer<MemoryService, Stack>
    {
    public:
        using Base = Layer<MemoryService, Stack>;

        using Base::above;
        using Base::below;

    public:
        static constexpr size_t REGION_MEMORY_CAPACITY    = 8ull << 30;
        static constexpr size_t TEMP_TEMP_TEMP_BLOCK_SIZE = 2 << 20;
        static constexpr size_t MIN_COMMITED_BLOCK_COUNT  = 16;
        static constexpr size_t MAX_COMMITED_BLOCK_COUNT  = 32;

        MemoryService();
        ~MemoryService();

        void start();
        void stop();

    public:
        Event& runnable_event() override final;

        void run() override final;

    private:
        bool is_undercommitted() const;
        bool is_overcommitted() const;

        void commit_block();
        void uncommit_block();

        void handle_madvise_result();
        void handle_madvise_free_result();
        void handle_madvise_populate_write_result();

        void update_readiness();

        std::span<std::byte> to_block_data(size_t block);

    private:
        enum class State {
            IDLE,
            PENDING_MADVISE_FREE,
            PENDING_MADVISE_POPULATE_WRITE,
        };

        State                  state_;
        bool                   pause_commits_;
        bool                   pause_uncommits_;
        Event*                 runnable_event_;
        Event                  activate_event_;

        std::span<std::byte>   storage_;
        MAdviseOperationHandle operation_;
        size_t                 transient_block_;
        std::vector<size_t>    committed_blocks_;
        std::vector<size_t>    uncommitted_blocks_;
    };

}

#include "memory_service.hpp"
