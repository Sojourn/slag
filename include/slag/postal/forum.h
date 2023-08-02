#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include "slag/bit_set.h"
#include "slag/spsc_queue.h"
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"

namespace slag::postal {

    struct alignas(64) ForumStatement {
        // How much we've consumed of the spsc queues from other participants
        std::vector<SpscQueueSequence>         consumer_sequences;

        // Changes in global buffers referenced by this Participant
        std::array<BitSet, BUFFER_GROUP_COUNT> changes;
    };

    // TODO: think about making this a template parameter on the forum,
    //       or possibly an interface
    struct ForumParticipant {
        // TODO: think about how much history we need
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
