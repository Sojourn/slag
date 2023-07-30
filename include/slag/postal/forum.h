#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include "slag/bit_set.h"
#include "slag/spsc_queue.h"
#include "slag/postal/types.h"

namespace slag::postal {

    struct alignas(64) ForumStatement {
        std::vector<SpscQueueSequence>         consumer_sequences;
        std::array<BitSet, BUFFER_GROUP_COUNT> borrowed_buffers;
    };

    struct ForumParticipant {
        // TODO: think about if we need 2 or 3 of these, which
        //       should be rounded up to the next power of 2 for
        //       efficient modulus'ing
        std::array<ForumStatement, 4> statements;
    };

    // used by participants (postmasters) to gossip about progress/resources
    class Forum {
    public:
        explicit Forum(size_t participant_count);

        // get the participant from this area
        ForumParticipant& participant(PostArea area);
        const ForumParticipant& participant(PostArea area) const;

        // get the statement made by the participant from this area in this epoch
        ForumStatement& statement(PostArea area, uint64_t epoch);
        const ForumStatement& statement(PostArea area, uint64_t epoch) const;

    private:
        std::vector<ForumParticipant> participants_;
    };

}
