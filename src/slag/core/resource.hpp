#include <cassert>

namespace slag {

    inline ResourceBase::ResourceBase(ResourceType type)
        : mantle::Object(static_cast<uint16_t>(type))
    {
    }

    inline ResourceType ResourceBase::resource_type() const {
        return static_cast<ResourceType>(user_data());
    }

    template<typename ResourceVisitor>
    void visit(ResourceVisitor&& visitor, ResourceBase& resource) {
        abort(); // TODO
    }

}
