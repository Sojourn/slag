#include "slag/postal/postal_service.h"
#include "slag/postal/post_office.h"
#include <limits>
#include <cassert>

namespace slag::postal {

    PostalService::PostalService(uint16_t identity)
        : identity_{identity}
    {
    }

    PostalService::~PostalService() {
        // sanity check that we outlived all of the local post offices
        for (const PostOffice* post_office: post_offices_) {
            assert(!post_office);
        }
    }

    uint16_t PostalService::identity() const {
        return identity_;
    }

    PostOffice& PostalService::post_office(PostArea area) {
        assert(area.service == identity_);
        assert(area.office < post_offices_.size());
        return *post_offices_[area.office];
    }

    PostArea PostalService::attach(PostOffice& post_office) {
        assert(post_offices_.size() <= std::numeric_limits<uint16_t>::max());

        PostArea area;
        area.service = identity_;
        area.office = static_cast<uint16_t>(post_offices_.size());
        post_offices_.push_back(&post_office);

        return area;
    }

    void PostalService::detach(PostOffice& post_office) {
        post_offices_[post_office.post_area().office] = nullptr;
    }

}
