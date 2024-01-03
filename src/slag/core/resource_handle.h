#pragma once

#include "slag/core/resource.h"

namespace slag {

    template<ResourceType type>
    class ResourceHandle {
        explicit ResourceHandle(Resource<type>& resource);

    public:
        ResourceHandle();
        ~ResourceHandle();

        ResourceHandle(ResourceHandle<type>&& other);
        ResourceHandle(const ResourceHandle<type>& other) = delete;
        ResourceHandle& operator=(ResourceHandle<type>&& rhs);
        ResourceHandle& operator=(const ResourceHandle<type>& rhs) = delete;

        explicit operator bool() const;

        Resource<type>& operator*();
        const Resource<type>& operator*() const;

        Resource<type>* operator->();
        const Resource<type>* operator->() const;

        ResourceHandle<type> share();
        void reset();

    private:
        void increment_reference_count();
        void decrement_reference_count();

    private:
        Resource<type>* resource_;
    };

}

#include "resource_handle.hpp"
