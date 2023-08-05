#pragma once

#include "slag/postal/forum.h"

namespace slag::postal {

    template<typename T>
    struct ForumStatement;

    class Custodian {
    public:
        explicit Custodian(Forum& forum)
            : forum_{forum}
        {
            // references_.resize(
            //     forum_.participant_count<PostMaster>()
            // );
        }

        // observe what other participants have said this epoch
        void listen(Forum& forum, uint64_t epoch) {
            for (size_t participant_index)
        }

        // prepare a statement for this epoch
        void speak(Forum& forum, uint64_t epoch) {
            // make a statement what buffers associated with each
            // PostMaster are unused
        }

    private:
        Forum&                        forum_;
        std::vector<BitSet>           references_;
        std::vector<BufferDescriptor> unused_buffers_;
    };

}
