#include "catch.hpp"
#include "slag/slag.h"
#include <emmintrin.h>

using namespace slag;

class Lru16 {
public:
    using Slot = int8_t;

    Lru16() {
        memset(ranking_, -1, sizeof(ranking_));
    }

    [[nodiscard]] std::pair<Slot, bool> allocate() {
        bool evicted = false;

        uint16_t slots = free_slots();
        if (!slots) {
            evicted = true;

            // decrement all rankings and recalculate the set of free slots
            __m128i old_ranking = _mm_load_si128(reinterpret_cast<const __m128i*>(ranking_));
            __m128i new_ranking = _mm_add_epi8(old_ranking, _mm_set1_epi8(static_cast<char>(-1)));
            _mm_store_si128(reinterpret_cast<__m128i*>(ranking_), new_ranking);

            slots = free_slots();
            assert(__builtin_popcount(slots) == 1);
        }

        Slot slot = static_cast<Slot>(__builtin_ctz(static_cast<unsigned int>(slots)));
        touch(slot);
        return std::make_pair(slot, evicted);
    }

    void deallocate(Slot slot) {
        assert((0 <= slot) && (slot < CAPACITY));

        ranking_[slot] = -1;
    }

    void touch(Slot slot) {
        assert((0 <= slot) && (slot < CAPACITY));

        int8_t old_slot_rank = ranking_[slot];
        int8_t new_slot_rank = MAX_RANK;
        if (old_slot_rank == new_slot_rank) {
            return; // no change in rank
        }

        // decrement all rankings
        __m128i old_ranking = _mm_load_si128(reinterpret_cast<const __m128i*>(ranking_));
        __m128i delta = _mm_set1_epi8(static_cast<char>(-1));
        __m128i new_ranking = _mm_add_epi8(old_ranking, delta);

        // store all non-saturated rankings
        __m128i saturation_slots = _mm_cmpgt_epi8(old_ranking, delta);
        _mm_maskmoveu_si128(new_ranking, saturation_slots, reinterpret_cast<char*>(ranking_));

        // give the slot the highest ranking
        ranking_[slot] = new_slot_rank;
    }

    [[nodiscard]] uint16_t used_slots() const {
        return static_cast<uint16_t>(
            _mm_movemask_epi8(
                _mm_cmpgt_epi8(
                    _mm_load_si128(reinterpret_cast<const __m128i*>(ranking_)),
                    _mm_setzero_si128()
                )
            )
        );
    }

    [[nodiscard]] uint16_t free_slots() const {
        return ~used_slots();
    }

    [[nodiscard]] size_t free_slot_count() const {
        return static_cast<size_t>(__builtin_popcount(free_slots()));
    }

    [[nodiscard]] size_t used_slot_count() const {
        return static_cast<size_t>(__builtin_popcount(used_slots()));
    }

    [[nodiscard]] bool is_used(Slot slot) const {
        return !is_free(slot);
    }

    [[nodiscard]] bool is_free(Slot slot) const {
        assert((0 <= slot) && (slot < CAPACITY));
        return ranking_[slot] < MIN_RANK;
    }

private:
    static constexpr int8_t CAPACITY = 16;
    static constexpr int8_t MAX_RANK = 15;
    static constexpr int8_t MIN_RANK = 0;

    alignas(16) int8_t ranking_[16];
};

TEST_CASE("tdd", "[Lru16]") {
    Lru16 lru;

    SECTION("allocation ordering") {
        for (size_t i = 0; i < 16; ++i) {
            CHECK(lru.allocate().first == static_cast<Lru16::Slot>(i));
        }
        for (size_t i = 0; i < 16; ++i) {
            CHECK(lru.allocate().first == static_cast<Lru16::Slot>(i));
        }

        lru.touch(0);
        for (size_t i = 0; i < 16; ++i) {
            CHECK(lru.allocate().first == static_cast<Lru16::Slot>((i + 1) % 16));
        }
    }
}
