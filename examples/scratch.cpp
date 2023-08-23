#include <iostream>
#include <cstdint>
#include <cstddef>
#include "slag/slag.h"

using namespace slag;

static constexpr size_t CHUNK_SIZE_BYTES        = (1 << 15) - 3;
static constexpr size_t BLOCK_SIZE_BYTES        = 2 << 20;
static constexpr size_t BLOCK_CHUNK_COUNT       = 64;
static constexpr size_t BLOCK_FOOTER_SIZE_BYTES = BLOCK_SIZE_BYTES - (BLOCK_CHUNK_COUNT * CHUNK_SIZE_BYTES);

struct Chunk {
    uint8_t storage[CHUNK_SIZE_BYTES];

    Chunk() {
        memset(storage, 0, sizeof(storage));
    }
};

struct BlockFooter {
    IntrusiveQueueNode node;
    uint64_t           mask;
    uint16_t           reference_counts[BLOCK_CHUNK_COUNT];

    BlockFooter()
        : mask{0}
    {
        memset(reference_counts, 0, sizeof(reference_counts));
    }
};

struct Block {
    Chunk       chunks[BLOCK_CHUNK_COUNT];
    BlockFooter footer;
    uint8_t     footer_reserved[BLOCK_FOOTER_SIZE_BYTES - sizeof(BlockFooter)];
};
static_assert(sizeof(Block) == BLOCK_SIZE_BYTES);

Block& to_block(const void* address) {
    uintptr_t block_mask = (BLOCK_SIZE_BYTES - 1);
    uintptr_t block_address = reinterpret_cast<uintptr_t>(address) & block_mask;
    return *reinterpret_cast<Block*>(block_address);
}

Chunk& to_chunk(const void* address) {
    Block& block = to_block(address);

    // Calculate how many bytes into the block this address is.
    uintptr_t block_offset = reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(&block);

    // Calculate how many chunks into the block this address is.
    size_t chunk_index = static_cast<uint32_t>(block_offset) / CHUNK_SIZE_BYTES;

    return block.chunks[chunk_index];
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Block block;
    (void)block;

    std::cout << sizeof(block) << std::endl;
    std::cout << sizeof(block.footer_reserved) << std::endl;

    return 0;
}
