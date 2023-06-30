// single buffer manager per-process
// single buffer group (for a given set of parameters) per-process
// per-thread buffer pools which access potentially multiple buffer groups

// update and compaction protocol for buffers?
// reference counting for buffers vs. probabalistic detection
// buffer pool configuration (pre-allocated buffers from various pools)
// buffer request/return ring buffers from the manager to pools
//   - groups internal to the manager?

// identifier for a buffer
struct BufferDescriptor {
    uint32_t group : 12; // (1ull << (group + 9)) -> (512, 2M)
    uint32_t index : 20; // ~1M
    uint32_t nonce : 31;
    uint32_t flags : 1;
};

class Block {
public:
    Block();
    explicit Block(size_t capacity);
    Block(Block&& other);
    Block(const Block&) = delete;
    ~Block();

    Block& operator=(Block&& rhs);
    Block& operator=(const Block&) = delete;

    explicit operator bool() const;

    [[nodiscard]] size_t size() const;
    [[nodiscard]] std::byte* data();
    [[nodiscard]] const std::byte* data() const;
    [[nodiscard]] std::span<std::byte> get();
    [[nodiscard]] std::span<const std::byte> get() const;

private:
    size_t size_;
    void*  data_;
};

// manages blocks of memory (and registers/unregisters)
// moves buffers between buffer groups (compacting)?
class BufferManager {
public:
    struct Config {
        size_t block_count;
        size_t block_size;
    };

    explicit BufferManager(const Config& config)
        : config_{config}
    {
        blocks_.reserve(config_.block_count);
    }

    ~BufferManager() {
    }

private:
    friend class BufferGroup;

private:
    Config                  config_;
    std::vector<BlockIndex> free_blocks_;
    std::vector<void*>      block_storage_; // block base pointers
};

// maintains attributes about buffers of similar (same size) buffers
class BufferGroup {
public:
    struct Config {
        size_t                buffer_size;
        size_t                minimum_buffer_count;
        std::optional<size_t> maximum_buffer_count;
    };

    BufferGroup(BufferManager& manager, const Config& config);

private:
    friend class BufferPool;
    friend class BufferHandle;
    friend class BufferAccessor;

    void increment_reference_count(BufferDescriptor descriptor);
    void decrement_reference_count(BufferDescriptor descriptor);

    void acquire_lock(BufferDescriptor descriptor);
    void release_lock(BufferDescriptor descriptor);

private:
    struct Buffer {
        BufferDescriptor descriptor;      // details about
        uint32_t         reference_count; // number of handles pointing to this buffer
        bool             locked;          // someone is exclusively accessing this piece of memory
        uint32_t         nonce;           // when the buffer was allocated / stale handle
    };
};

// used to allocate individual buffers from a pre-allocated pool
class BufferPool {
public:
};

// stores a reference to a buffer
class BufferHandle {
public:
    BufferHandle();
    explicit BufferHandle(BufferDescriptor descriptor);
    BufferHandle(BufferHandle&& other);
    BufferHandle(const BufferHandle& other) = delete;
    ~BufferHandle();

    BufferHandle& operator=(BufferHandle&& other);
    BufferHandle& operator=(const BufferHandle& other) = delete;

    explicit operator bool() const {
        return static_cast<bool>(descriptor_);
    }

    // may fail to acquire the lock
    [[nodiscard]] BufferAccessor lock() {
        if (!descriptor_) {
            throw std::runtime_error("Invalid BufferHandle");
        }

        return {
            *descriptor_
        };
    }

private:
    std::optional<BufferDescriptor> descriptor_;
};

enum BufferAccess : uint8_t {
    NONE,
    READ,
    READ_WRITE,
};

// indicates that the buffer is being written/read
// pins the memory
// exclusive access
class BufferAccessor {
public:
    BufferAccessor();
    BufferAccessor(BufferDescriptor descriptor, BufferAccess access=BufferAccess::READ_WRITE);
    BufferAccessor(BufferAccessor&& other);
    BufferAccessor(const BufferAccessor& other) = delete;
    ~BufferAccessor();

    BufferAccessor& operator=(BufferAccessor&& other);
    BufferAccessor& operator=(const BufferAccessor& other) = delete;

    [[nodiscard]] std::span<std::byte> get();
    [[nodiscard]] std::span<const std::byte> get() const;

    [[nodiscard]] size_t size() const;
    [[nodiscard]] std::byte* data();
    [[nodiscard]] const std::byte* data() const;

private:
    std::optional<BufferDescriptor> descriptor_;
    BufferAccess                    access_;
};
