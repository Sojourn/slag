#pragma once

#include <vector>
#include "slag/postal/types.h"

namespace slag::postal {

    class PostalService {
        PostalService(PostalService&&) = delete;
        PostalService(const PostalService&) = delete;
        PostalService& operator=(PostalService&&) = delete;
        PostalService& operator=(const PostalService&) = delete;

    public:
        explicit PostalService(uint16_t identity);
        ~PostalService();

        uint16_t identity() const;
        PostOffice& post_office(PostArea area);

    private:
        friend class PostOffice;

        PostArea attach(PostOffice& post_office);
        void detach(PostOffice& post_office);

    private:
        uint16_t                 identity_;
        std::vector<PostOffice*> post_offices_;
    };

}
