#include <cassert>
#include <stdexcept>

namespace slag {

    template<typename ServiceImpl, typename... Args>
    inline void ServiceRegistry::initialize(Args&&... args) {
        std::unique_ptr<Service> service = std::make_unique<ServiceImpl>(std::forward<Args>(args)...);
        services_[to_index(service->type())] = std::move(service);
    }

    template<ServiceType type>
    inline ServiceInterface<type>& ServiceRegistry::get_service() {
        return static_cast<ServiceInterface<type>&>(*services_[to_index(type)]);
    }

    inline Service& ServiceRegistry::get_service(ServiceType type) {
        return *services_[to_index(type)];
    }

    template<ServiceType type>
    inline const ServiceInterface<type>& ServiceRegistry::get_service() const {
        return static_cast<const ServiceInterface<type>&>(*services_[to_index(type)]);
    }

    inline const Service& ServiceRegistry::get_service(ServiceType type) const {
        return *services_[to_index(type)];
    }

    inline void ServiceRegistry::start_services() {
        for (size_t i = 0; i < services_.size(); ++i) {
            auto&& service = services_[i];
            assert(service);
            service->start_service();
        }
    }

    inline void ServiceRegistry::stop_services() {
        for (size_t i = 0; i < services_.size(); ++i) {
            auto&& service = services_[services_.size() - i - 1];
            assert(service);
            service->stop_service();
        }
    }

}
