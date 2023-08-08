#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/queue.h"
#include "slag/postal/types.h"

namespace slag::postal {

    class Region;

    class PostOffice {
        PostOffice(PostOffice&&) = delete;
        PostOffice(const PostOffice&) = delete;
        PostOffice& operator=(PostOffice&&) = delete;
        PostOffice& operator=(const PostOffice&) = delete;

    public:
        explicit PostOffice(Region& region);
        ~PostOffice();

        PostArea post_area() const;
        PostBox* post_box(PostCode post_code);

    private:
        friend class PostBox;

        PostCode attach(PostBox& post_box);
        void detach(PostBox& post_box);

    private:
        Region&               region_;
        PostArea              post_area_;
        std::vector<PostBox*> post_boxes_;
        Queue<uint32_t>       unused_post_box_numbers_;
    };

}
