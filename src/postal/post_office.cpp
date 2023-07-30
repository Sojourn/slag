#include "slag/postal/post_office.h"
#include "slag/postal/post_box.h"
#include "slag/postal/postal_service.h"
#include <cassert>

namespace slag::postal {

    PostOffice::PostOffice(PostalService& postal_service)
        : postal_service_{postal_service}
        , post_area_{postal_service.attach(*this)}
    {
    }

    PostOffice::~PostOffice() {
        // sanity check that we outlived all of the local post boxes
        assert(post_boxes_.size() == unused_post_box_numbers_.size());

        postal_service_.detach(*this);
    }

    PostBox* PostOffice::post_box(PostCode post_code) {
        if (post_area_ != post_code) {
            return nullptr;
        }
        if (post_boxes_.size() <= post_code.number) {
            return nullptr;
        }

        return post_boxes_[post_code.number];
    }

    PostCode PostOffice::attach(PostBox& post_box) {
        PostCode post_code;
        static_cast<PostArea&>(post_code) = post_area_;

        if (!unused_post_box_numbers_.empty()) {
            post_code.number = unused_post_box_numbers_.back();
            unused_post_box_numbers_.pop_back();
        }
        else {
            post_code.number = static_cast<uint32_t>(post_boxes_.size());
            post_boxes_.push_back(&post_box);
        }

        return post_code;
    }

    void PostOffice::detach(PostBox& post_box) {
        uint32_t number = post_box.post_code().number;
        post_boxes_[number] = nullptr;
        unused_post_box_numbers_.push_back(number);
    }

}
