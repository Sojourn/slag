#pragma once

#include "slag/queue.h"
#include "slag/postal/types.h"
#include "slag/postal/envelope.h"

namespace slag::postal {

    class PostBox {
        PostBox(PostBox&&) = delete;
        PostBox(const PostBox&) = delete;
        PostBox& operator=(PostBox&&) = delete;
        PostBox& operator=(const PostBox&) = delete;

    public:
        explicit PostBox(PostOffice& post_office);
        ~PostBox();

        PostOffice& post_office();
        PostArea post_area() const;
        PostCode post_code() const;

        // send an envelope with these contents to a mailbox
        void send(Envelope envelope);

        // get the next incoming envelope from the mailbox
        std::optional<Envelope> receive();

    private:
        friend class PostMaster;

        Queue<Envelope>& incoming();
        Queue<Envelope>& outgoing();

    private:
        PostOffice&     post_office_;
        PostCode        post_code_;
        Queue<Envelope> incoming_;
        Queue<Envelope> outgoing_;
    };

}
