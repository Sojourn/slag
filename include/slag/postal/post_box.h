#pragma once

#include "slag/postal/types.h"

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

    private:
        friend class PostMaster;

        // TODO: functions that do last mile pickup+delivery for the post master to use

    private:
        PostOffice& post_office_;
        PostCode    post_code_;
    };

}
