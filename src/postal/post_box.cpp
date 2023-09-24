#include "slag/postal/post_box.h"
#include "slag/postal/post_office.h"

namespace slag {

    PostBox::PostBox(PostOffice& post_office)
        : post_office_{post_office}
        , post_code_{post_office.attach(*this)}
    {
    }

    PostBox::~PostBox() {
        post_office_.detach(*this);
    }

    PostOffice& PostBox::post_office() {
        return post_office_;
    }

    PostArea PostBox::post_area() const {
        return post_code_;
    }

    PostCode PostBox::post_code() const {
        return post_code_;
    }

    PollableQueue<Envelope>& PostBox::incoming_queue() {
        return incoming_queue_;
    }

    PollableQueue<Envelope>& PostBox::outgoing_queue() {
        return outgoing_queue_;
    }

    void PostBox::send(Envelope envelope) {
        outgoing_queue_.push_back(std::move(envelope));
    }

    std::optional<Envelope> PostBox::receive() {
        return incoming_queue_.pop_front();
    }

}
