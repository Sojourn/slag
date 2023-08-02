#include <cassert>

namespace slag::postal {

    inline ForumCensus::ForumCensus() {
        memset(participant_counts_, 0, sizeof(participant_counts_));
    }

    inline size_t ForumCensus::participant_count(ForumParticipantType participant_type) const {
        return participant_counts_[static_cast<size_t>(participant_type)];
    }

    inline void ForumCensus::set_participant_count(ForumParticipantType participant_type, size_t count) {
        participant_counts_[static_cast<size_t>(participant_type)] = count;
    }

    inline Forum::Forum(const ForumCensus& census)
        : census_{census}
    {
#define X(SLAG_POSTAL_FORUM_PARICIPANT_TYPE)                                                                                   \
        std::get<static_cast<size_t>(ForumParticipantType::SLAG_POSTAL_FORUM_PARICIPANT_TYPE)>(recent_statements_).resize( \
            participant_count(ForumParticipantType::SLAG_POSTAL_FORUM_PARICIPANT_TYPE)                                     \
        );                                                                                                                 \

SLAG_POSTAL_FORUM_PARICIPANT_TYPES(X)
#undef X
    }

    inline const ForumCensus& Forum::census() const {
        return census_;
    }

    inline size_t Forum::participant_count(ForumParticipantType participant_type) const {
        return census_.participant_count(participant_type);
    }

    template<ForumParticipantType type>
    inline ForumStatement<type>& Forum::statement(size_t participant_index, uint64_t epoch) {
        return std::get<static_cast<size_t>(type)>(recent_statements_)[participant_index][HISTORY_MASK & epoch];
    }

    template<ForumParticipantType type>
    inline const ForumStatement<type>& Forum::statement(size_t participant_index, uint64_t epoch) const {
        return std::get<static_cast<size_t>(type)>(recent_statements_)[participant_index][HISTORY_MASK & epoch];
    }

}
