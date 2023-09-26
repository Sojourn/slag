#include <cassert>

namespace slag {

    template<typename Stack>
    inline MemoryService<Stack>::MemoryService()
        : Service{ServiceType::MEMORY}
        , state_{State::IDLE}
        , pause_commits_{false}
        , pause_uncommits_{false}
        , runnable_event_{nullptr}
        , transient_block_{0}
    {
        int file_descriptor = -1;
        off_t file_offset = 0;

        void* storage_address = ::mmap(
            NULL,
            REGION_MEMORY_CAPACITY,
            PROT_READ|PROT_WRITE,
            MAP_PRIVATE|MAP_ANONYMOUS,
            file_descriptor,
            file_offset
        );
        if (storage_address == MAP_FAILED) {
            throw std::runtime_error("Failed to allocate memory for region");
        }

        std::byte* storage_beg = reinterpret_cast<std::byte*>(storage_address);
        std::byte* storage_end = storage_beg + REGION_MEMORY_CAPACITY;

        // Align storage, and free the unaligned parts.
        {
            std::byte* aligned_storage_beg = align_forward<TEMP_TEMP_TEMP_BLOCK_SIZE>(storage_beg);
            std::byte* aligned_storage_end = align_backward<TEMP_TEMP_TEMP_BLOCK_SIZE>(storage_end);

            assert(aligned_storage_beg < aligned_storage_end);
            assert(aligned_storage_end - aligned_storage_beg <= storage_end - storage_beg);

            if (storage_beg < aligned_storage_beg) {
                int status = ::munmap(storage_beg, aligned_storage_beg - storage_beg);
                assert(status >= 0);
            }
            if (aligned_storage_end < storage_end) {
                int status = ::munmap(aligned_storage_end, storage_end - aligned_storage_end);
                assert(status >= 0);
            }

            storage_ = std::span {
                aligned_storage_beg,
                static_cast<size_t>(aligned_storage_end - aligned_storage_beg)
            };
        }

        // Initialize block collections.
        {
            size_t block_count = storage_.size_bytes() / TEMP_TEMP_TEMP_BLOCK_SIZE;

            committed_blocks_.reserve(block_count);
            uncommitted_blocks_.reserve(block_count);

            for (size_t block_index = 0; block_index < block_count; ++block_index) {
                uncommitted_blocks_.push_back(block_count - block_index - 1);
            }
        }
    }

    template<typename Stack>
    inline MemoryService<Stack>::~MemoryService() {
        int status = ::munmap(storage_.data(), storage_.size_bytes());
        assert(status >= 0);
    }

    template<typename Stack>
    inline void MemoryService<Stack>::start() {
        info("[MemoryService] starting");

        set_service_state(ServiceState::STARTING);

        update_readiness();
    }

    template<typename Stack>
    inline void MemoryService<Stack>::stop() {
        info("[MemoryService] stopping");

        set_service_state(ServiceState::STOPPING);

        // Make any pending operation finish sooner rather than later.
        if (operation_) {
            operation_->cancel();
        }

        update_readiness();
    }

    template<typename Stack>
    inline Event& MemoryService<Stack>::runnable_event() {
        return *runnable_event_;
    }

    template<typename Stack>
    inline void MemoryService<Stack>::run() {
        if (is_service_starting()) {
            set_service_state(ServiceState::RUNNING);
            return;
        }

        // Check if there is an operation result to consume.
        if (operation_ && operation_->is_complete()) {
            handle_madvise_result();
        }

        // Stop if we are now quiescent.
        if (!operation_ && is_service_stopping()) {
            set_success();
            set_service_state(ServiceState::STOPPED);

            info("[MemoryService] stopped");
            return;
        }

        // Kick off an operation if we aren't in the sweet spot.
        if (is_undercommitted()) {
            commit_block();
        }
        else if (is_overcommitted()) {
            uncommit_block();
        }

        update_readiness();
    }

    template<typename Stack>
    inline bool MemoryService<Stack>::is_undercommitted() const {
        if (uncommitted_blocks_.empty()) {
            return false; // We cannot commit any more .
        }

        return committed_blocks_.size() < MIN_COMMITED_BLOCK_COUNT;
    }

    template<typename Stack>
    inline bool MemoryService<Stack>::is_overcommitted() const {
        if (committed_blocks_.empty()) {
            return false; // We cannot commit any less.
        }

        return committed_blocks_.size() > MAX_COMMITED_BLOCK_COUNT;
    }

    template<typename Stack>
    inline void MemoryService<Stack>::commit_block() {
        assert(!uncommitted_blocks_.empty());
        transient_block_ = uncommitted_blocks_.back();
        uncommitted_blocks_.pop_back();

        std::span<std::byte> block_data = to_block_data(transient_block_);
        operation_ = make_madvise_operation(
            block_data.data(),
            block_data.size(),
            MADV_POPULATE_WRITE
        );

        state_ = State::PENDING_MADVISE_POPULATE_WRITE;
    }

    template<typename Stack>
    inline void MemoryService<Stack>::uncommit_block() {
        assert(!committed_blocks_.empty());
        transient_block_ = committed_blocks_.back();
        committed_blocks_.pop_back();

        std::span<std::byte> block_data = to_block_data(transient_block_);
        operation_ = make_madvise_operation(
            block_data.data(),
            block_data.size(),
            MADV_FREE
        );

        state_ = State::PENDING_MADVISE_FREE;
    }

    template<typename Stack>
    inline void MemoryService<Stack>::handle_madvise_result() {
        assert(operation_ && operation_->is_complete());

        switch (state_) {
            case State::IDLE: {
                abort();
                break;
            }
            case State::PENDING_MADVISE_FREE: {
                handle_madvise_free_result();
                break;
            }
            case State::PENDING_MADVISE_POPULATE_WRITE: {
                handle_madvise_populate_write_result();
                break;
            }
        }

        state_ = State::IDLE;
        operation_.reset();
    }

    template<typename Stack>
    inline void MemoryService<Stack>::handle_madvise_free_result() {
        auto&& result = operation_->result();

        if (result) {
            info("[MemoryService] block:{} uncommitted", transient_block_);
            uncommitted_blocks_.push_back(transient_block_);
        }
        else {
            assert(false);
            pause_uncommits_ = true;
            committed_blocks_.push_back(transient_block_);
        }
    }

    template<typename Stack>
    inline void MemoryService<Stack>::handle_madvise_populate_write_result() {
        auto&& result = operation_->result();

        if (result) {
            info("[MemoryService] block:{} committed", transient_block_);
            committed_blocks_.push_back(transient_block_);
        }
        else {
            assert(false);
            pause_commits_ = true;
            uncommitted_blocks_.push_back(transient_block_);
        }
    }

    template<typename Stack>
    inline void MemoryService<Stack>::update_readiness() {
        if (operation_) {
            runnable_event_ = &operation_->complete_event();
        }
        else {
            switch (service_state()) {
                case ServiceState::STOPPING: {
                    activate_event_.set();
                    runnable_event_ = &activate_event_;
                    break;
                }
                default: {
                    bool activate = false;
                    activate |= (!pause_commits_ && is_undercommitted());
                    activate |= (!pause_uncommits_ && is_overcommitted());

                    activate_event_.set(activate);
                    runnable_event_ = &activate_event_;
                    break;
                }
            }
        }
    }

    template<typename Stack>
    inline std::span<std::byte> MemoryService<Stack>::to_block_data(size_t block) {
        return storage_.subspan(block * TEMP_TEMP_TEMP_BLOCK_SIZE, TEMP_TEMP_TEMP_BLOCK_SIZE);
    }

}
