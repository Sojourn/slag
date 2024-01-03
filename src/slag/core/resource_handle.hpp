#include "slag/context.h"
#include "slag/core/resource_table.h"

namespace slag {

    template<ResourceType type>
    inline ResourceHandle<type>::ResourceHandle(Resource<type>& resource)
        : resource_(&resource)
    {
    }

    template<ResourceType type>
    inline ResourceHandle<type>::ResourceHandle()
        : resource_(nullptr)
    {
    }

    template<ResourceType type>
    inline ResourceHandle<type>::~ResourceHandle() {
        reset();
    }

    template<ResourceType type>
    inline ResourceHandle<type>::ResourceHandle(ResourceHandle<type>&& other)
        : resource_(std::exchange(other.resource_, nullptr))
    {
    }

    template<ResourceType type>
    inline ResourceHandle<type>& ResourceHandle<type>::operator=(ResourceHandle<type>&& rhs) {
        if (this != &rhs) {
            reset();

            resource_ = std::exchange(rhs.resource_, nullptr);
        }

        return *this;
    }

    template<ResourceType type>
    inline ResourceHandle<type>::operator bool() const {
        return static_cast<bool>(resource_);
    }

    template<ResourceType type>
    inline Resource<type>& ResourceHandle<type>::operator*() {
        assert(resource_);
        return *resource_;
    }

    template<ResourceType type>
    inline const Resource<type>& ResourceHandle<type>::operator*() const {
        assert(resource_);
        return *resource_;
    }

    template<ResourceType type>
    inline Resource<type>* ResourceHandle<type>::operator->() {
        return resource_;
    }

    template<ResourceType type>
    inline const Resource<type>* ResourceHandle<type>::operator->() const {
        return resource_;
    }

    template<ResourceType type>
    inline ResourceHandle<type> ResourceHandle<type>::share() {
        if (!resource_) {
            return {};
        }

        increment_reference_count();
        return {*resource_};
    }

    template<ResourceType type>
    inline void ResourceHandle<type>::reset() {
        if (!resource_) {
            return;
        }

        decrement_reference_count();
        resource_ = nullptr;
    }

    template<ResourceType type>
    inline void ResourceHandle<type>::increment_reference_count() {
        Context& context = get_context();
        ResourceDescriptor descriptor = resource_->descriptor();
        ResourceTable& table = context.resource_table(descriptor.type);

        table.increment_reference_count(*this);
    }

    template<ResourceType type>
    inline void ResourceHandle<type>::decrement_reference_count() {
        Context& context = get_context();
        ResourceDescriptor descriptor = resource_->descriptor();
        ResourceTable& table = context.resource_table(descriptor.type);

        table.decrement_reference_count(*this);
    }

}
