#pragma once

#include "slag/object.h"
#include "slag/resource.h"

namespace slag {

    template<>
    class Resource<ResourceType::MANAGED> : public Object {
    public:
        Resource()
            : Object(static_cast<ObjectGroup>(ResourceType::MANAGED))
        {
        }
    };

}
