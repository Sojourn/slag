#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include "slag/bit_set.h"
#include "slag/spsc_queue.h"
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"

#define SLAG_POSTAL_FORUM_PARICIPANT_TYPES(X) \
    X(POST_MASTER_GENERAL)                    \
    X(POST_MASTER)                            \
    X(CUSTODIAN)                              \

namespace slag::postal {

    enum class ForumParticipantType {
#define X(SLAG_POSTAL_FORUM_PARICIPANT_TYPE) SLAG_POSTAL_FORUM_PARICIPANT_TYPE,
    SLAG_POSTAL_FORUM_PARICIPANT_TYPES(X)

#undef X
    };

    constexpr size_t FORUM_PARTICIPANT_TYPE_COUNT = 0
#define X(SLAG_POSTAL_FORUM_PARICIPANT_TYPE) + 1
    SLAG_POSTAL_FORUM_PARICIPANT_TYPES(X)
#undef X
    ;

    template<ForumParticipantType participant_type>
    struct ForumStatement;

    template<>
    struct alignas(64) ForumStatement<ForumParticipantType::POST_MASTER_GENERAL> {
    };

    template<>
    struct alignas(64) ForumStatement<ForumParticipantType::POST_MASTER> {
        std::vector<SpscQueueSequence> sequences;
        BitSet                         global_buffer_ownership_delta;
        BitSet                         global_buffer_reference_delta;
    };

    template<>
    struct alignas(64) ForumStatement<ForumParticipantType::CUSTODIAN> {
    };

    class ForumCensus {
    public:
        ForumCensus();

        size_t participant_count(ForumParticipantType participant_type) const;
        void set_participant_count(ForumParticipantType participant_type, size_t count);

    private:
        size_t participant_counts_[FORUM_PARTICIPANT_TYPE_COUNT];
    };

    // used by participants (postmasters) to gossip about progress/resources
    class Forum {
    public:
        static constexpr size_t HISTORY      = 4;
        static constexpr size_t HISTORY_MASK = HISTORY - 1;

        explicit Forum(const ForumCensus& census);

        const ForumCensus& census() const;
        size_t participant_count(ForumParticipantType participant_type) const;

        template<ForumParticipantType type>
        ForumStatement<type>& statement(size_t participant_index, uint64_t epoch);

        template<ForumParticipantType type>
        const ForumStatement<type>& statement(size_t participant_index, uint64_t epoch) const;

    private:
        using RecentStatements = std::tuple<
#define X(SLAG_POSTAL_FORUM_PARICIPANT_TYPE)                                  \
            std::vector<                                                      \
                std::array<ForumStatement<                                    \
                    ForumParticipantType::SLAG_POSTAL_FORUM_PARICIPANT_TYPE>, \
                    FORUM_PARTICIPANT_TYPE_COUNT                              \
                >                                                             \
            >,                                                                \

            SLAG_POSTAL_FORUM_PARICIPANT_TYPES(X)
#undef X
            int
        >;

        ForumCensus      census_;
        RecentStatements recent_statements_;
    };

}

#include "forum.hpp"
