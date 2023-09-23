#pragma once

#include "slag/collection/queue.h"
#include "slag/collection/intrusive_list.h"
#include "slag/postal/types.h"
#include "slag/postal/domain.h"
#include "slag/postal/envelope.h"
#include "slag/postal/pollable.h"
#include "slag/postal/pollable_queue.h"

namespace slag::postal {

    class PostBox {
        PostBox(PostBox&&) = delete;
        PostBox(const PostBox&) = delete;
        PostBox& operator=(PostBox&&) = delete;
        PostBox& operator=(const PostBox&) = delete;

    public:
        explicit PostBox(PostOffice& post_office = region().post_office());
        ~PostBox();

        PostOffice& post_office();
        PostArea post_area() const;
        PostCode post_code() const;
        PollableQueue<Envelope>& incoming_queue();
        PollableQueue<Envelope>& outgoing_queue();

        // send an envelope with these contents to a mailbox
        void send(Envelope envelope);

        // get the next incoming envelope from the mailbox
        std::optional<Envelope> receive();

    private:
        PostOffice&             post_office_;
        PostCode                post_code_;
        PollableQueue<Envelope> incoming_queue_;
        PollableQueue<Envelope> outgoing_queue_;
    };

}
