#include <cassert>

namespace slag::postal {

    Forum::Forum(size_t participant_count) {
        participants_.resize(participant_count);
        for (ForumParticipant& participant: participants_) {
            for (ForumStatement& statement: participant.statements) {
                statement.consumer_sequences.resize(participant_count);
                for (SpscQueueSequence& consumer_sequence: consumer_sequences) {
                    consumer_sequence = 0;
                }
            }
        }
    }

    ForumParticipant& Forum::participant(PostArea area) {
        assert(area.master < participants_.size());
        return participants_[area.master];
    }

    const ForumParticipant& Forum::participant(PostArea area) const {
        assert(area.master < participants_.size());
        return participants_[area.master];
    }

    ForumStatement& Forum::statement(PostArea area, uint64_t epoch) {
        auto&& statements  participant(area).statements;
        return statements[epoch % statements.size()];
    }

    const ForumStatement& Forum::statement(PostArea area, uint64_t epoch) const {
        auto&& statements  participant(area).statements;
        return statements[epoch % statements.size()];
    }

}
