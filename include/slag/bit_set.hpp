#include <limits>
#include <cassert>

namespace slag {

    inline BitSet::BitSet(size_t size_bits)
        : size_bits_{0}
    {
        grow_size_bits(size_bits);
    }

    inline bool BitSet::test(size_t index) const {
        assert(index < size_bits_);

        size_t block_offset = to_block_offset(index);
        size_t bit_offset = to_bit_offset(index);

        return blocks_[block_offset] & (1ull << bit_offset);
    }

    inline void BitSet::set() {
        size_t block_count = blocks_.size();
        if (!block_count) {
            return;
        }

        // completely set all blocks except for the last
        for (size_t block_index = 0; block_index < (block_count - 1); ++block_index) {
            blocks_[block_index] = std::numeric_limits<uint64_t>::max();
        }

        // set bits in the last, potentially partially aligned block
        auto partial_block_size_bits = size_bits_ & (BLOCK_SIZE_BITS - 1);
        auto partial_block_mask = (1ull << partial_block_size_bits) - 1;
        blocks_.back() |= partial_block_mask;
    }

    inline void BitSet::set(size_t index, bool value) {
        assert(index < size_bits_);

        if (value) {
            size_t block_offset = to_block_offset(index);
            size_t bit_offset = to_bit_offset(index);

            blocks_[block_offset] |= (1ull << bit_offset);
        }
        else {
            reset(index);
        }
    }

    inline void BitSet::reset() {
        for (Block& block: blocks_) {
            block = 0;
        }
    }

    inline void BitSet::reset(size_t index) {
        assert(index < size_bits_);

        size_t block_offset = to_block_offset(index);
        size_t bit_offset = to_bit_offset(index);

        blocks_[block_offset] &= ~(1ull << bit_offset);
    }


    inline size_t BitSet::size_bits() const {
        return size_blocks() / BLOCK_SIZE_BITS;
    }

    inline size_t BitSet::size_bytes() const {
        return size_blocks() / BLOCK_SIZE_BYTES;
    }

    inline size_t BitSet::size_blocks() const {
        return blocks_.size();
    }

    inline void BitSet::grow_size_bits(size_t new_size_bits) {
        if (new_size_bits < size_bits_) {
            assert(false); // we would shrink
            return;
        }
        else if (new_size_bits == size_bits_) {
            return; // would not grow
        }

        size_t old_block_count = blocks_.size();
        size_t new_block_count = calculate_block_count(new_size_bits);

        blocks_.resize(new_block_count);
        for (size_t block_index = old_block_count; block_index < new_block_count; ++block_index) {
            blocks_[block_index] = 0; // just in case these aren't being initialized to zero... 
        }

        size_bits_ = new_size_bits;
    }

    inline void BitSet::grow_size_bytes(size_t new_size_bytes) {
        grow_size_blocks(new_size_bytes * BLOCK_SIZE_BYTES);
    }

    inline void BitSet::grow_size_blocks(size_t new_size_blocks) {
        grow_size_bits(new_size_blocks * BLOCK_SIZE_BITS);
    }

    inline auto BitSet::block(size_t block_index) -> Block& {
        return blocks_[block_index];
    }

    inline auto BitSet::block(size_t block_index) const -> const Block& {
        return blocks_[block_index];
    }

    inline size_t BitSet::calculate_block_count(size_t size_bits) {
        size_t base_block_count = size_bits / BLOCK_SIZE_BITS;
        size_t block_count = base_block_count;

        // include a padding block there are a non-zero number of padding bits
        if (calculate_padding_bit_count(size_bits) > 0) {
            block_count += 1;
        }

        return block_count;
    }

    inline size_t BitSet::calculate_padding_bit_count(size_t size_bits) {
        size_t non_padding_bit_count = size_bits & (BLOCK_SIZE_BITS - 1);

        return BLOCK_SIZE_BITS - non_padding_bit_count - 1;
    }

    inline size_t BitSet::to_block_offset(size_t index) {
        return index >> BLOCK_SIZE_BITS_LOG2;
    }

    inline size_t BitSet::to_bit_offset(size_t index) {
        return index & (BLOCK_SIZE_BITS - 1);
    }

    inline BitSetScanner::BitSetScanner(const BitSet& bit_set)
        : bit_set_{&bit_set}
        , next_block_index_{0}
        , block_popcount_{0}
        , block_{0}
    {
    }

    inline std::optional<size_t> BitSetScanner::next() {
        // iterate over blocks until we find one that has set bits
        while (block_popcount_ == 0) {
            if (!fetch_block()) {
                return std::nullopt; // reached the end
            }
        }

        // updating block_popcount_ has a much shorter data dependency chain than block_,
        // which is why we use it as the loop invariant
        block_popcount_ -= 1;

        // find the next bit and calculate offsets
        auto current_block_offset = next_block_index_ - 1;
        auto block_offset = current_block_offset * BLOCK_SIZE_BITS;
        auto bit_offset = static_cast<size_t>(__builtin_ctzll(block_));

        // remove this bit from our internal state
        auto bit_mask = 1ull << bit_offset;
        block_ &= ~bit_mask;

        return block_offset + bit_offset;
    }

    inline bool BitSetScanner::fetch_block() {
        if (next_block_index_ >= bit_set_->size_blocks()) {
            return false;
        }

        // advance to the next block
        block_ = bit_set_->block(next_block_index_);
        next_block_index_ += 1;

        // count the number of set bits in this block
        block_popcount_ = static_cast<size_t>(__builtin_popcountll(block_));

        return true;
    }

}
