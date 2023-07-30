#include "slag/postal/post_box.h"
#include "slag/postal/post_office.h"

namespace slag::postal {

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

}
