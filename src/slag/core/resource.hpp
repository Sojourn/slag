#include <cassert>

namespace slag {

    inline ResourceBase::ResourceBase(ResourceDescriptor descriptor)
        : descriptor_(descriptor)
    {
    }

    inline ResourceType ResourceBase::type() const {
        return descriptor_.type;
    }

    inline ResourceDescriptor ResourceBase::descriptor() const {
        return descriptor_;
    }

}
