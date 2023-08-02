#pragma once

#include "slag/postal/types.h"
#include "slag/postal/buffer.h"

namespace slag::postal {

    class Envelope {
    public:
        const PostCode& to() const;
        const PostCode& from() const;
        const PostageStamp& stamp() const;
        const BufferHandle& content() const;

        void set_to(PostCode post_code);
        void set_from(PostCode post_code);
        void set_stamp(PostageStamp stamp);
        void set_content(BufferHandle content);

    private:
        PostCode     to_;
        PostCode     from_;
        PostageStamp stamp_;
        BufferHandle content_;
    };

}

#include "envelope.hpp"
