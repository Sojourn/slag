#pragma once

#include <type_traits>
#include <optional>
#include <cstdint>
#include <cstddef>
#include "slag/util.h"

namespace slag {

    class BitSet {
    public:
        using Block = uint64_t;

        static constexpr size_t BLOCK_SIZE_BITS      = SIZE_BITS<Block>;
        static constexpr size_t BLOCK_SIZE_BYTES     = SIZE_BYTES<Block>;
        static constexpr size_t BLOCK_SIZE_BITS_LOG2 = 6;

        static_assert(1ull << BLOCK_SIZE_BITS_LOG2 == BLOCK_SIZE_BITS, "exp2(BLOCK_SIZE_BITS_LOG2) must equal BLOCK_SIZE_BITS");

    public:
        explicit BitSet(size_t size_bits = 0);

        size_t size_bits() const;
        size_t size_bytes() const;
        size_t size_blocks() const;

        bool any() const;
        bool none() const;
        bool test(size_t index) const;

        void set();
        void set(size_t index, bool value=true);

        void reset();
        void reset(size_t index);

        void grow_size_bits(size_t new_size_bits);
        void grow_size_bytes(size_t new_size_bytes);
        void grow_size_blocks(size_t new_size_blocks);

        Block& block(size_t block_index);
        const Block& block(size_t block_index) const;

    private:
        // calculate how many blocks we need for an array with this many bits
        static size_t calculate_block_count(size_t size_bits);

        // calculate how many padding bits are in the last block
        static size_t calculate_padding_bit_count(size_t size_bits);

        // the offset of a block within an array of blocks
        static size_t to_block_offset(size_t index);

        // the offset of a bit within a block
        static size_t to_bit_offset(size_t index);

    private:
        std::vector<Block> blocks_;
        size_t             size_bits_;
    };

    class BitSetScanner {
    public:
        using Block = BitSet::Block;

        static constexpr size_t BLOCK_SIZE_BITS  = BitSet::BLOCK_SIZE_BITS;
        static constexpr size_t BLOCK_SIZE_BYTES = BitSet::BLOCK_SIZE_BYTES;

    public:
        explicit BitSetScanner(const BitSet& bit_set);

        std::optional<size_t> next();

    private:
        bool fetch_block();

    private:
        const BitSet* bit_set_;
        size_t        next_block_index_;
        size_t        block_popcount_;
        BitSet::Block block_;
    };

}

#include "bit_set.hpp"
