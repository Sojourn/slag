#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"

namespace slag::postal {

    class PostOffice {
        PostOffice(PostOffice&&) = delete;
        PostOffice(const PostOffice&) = delete;
        PostOffice& operator=(PostOffice&&) = delete;
        PostOffice& operator=(const PostOffice&) = delete;

    public:
        explicit PostOffice(PostalService& postal_service);
        ~PostOffice();

        PostArea post_area() const;
        PostBox* post_box(PostCode post_code);

    private:
        friend class PostBox;

        PostCode attach(PostBox& post_box);
        void detach(PostBox& post_box);

    private:
        PostalService&        postal_service_;
        PostArea              post_area_;
        std::vector<PostBox*> post_boxes_;

        // TODO: think about using a queue instead of a stack for these
        std::vector<uint32_t> unused_post_box_numbers_;
    };

}
