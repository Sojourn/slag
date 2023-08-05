#include "slag/postal/domain.h"

namespace slag::postal {

    static Domain<DomainType::EMPIRE>* empire_instance = nullptr;
    static Domain<DomainType::NATION>* nation_instance = nullptr;
    thread_local Domain<DomainType::REGION>* region_instance = nullptr;

    template<>
    Domain<DomainType::EMPIRE>& domain() {
        return *empire_instance;
    }

    template<>
    Domain<DomainType::NATION>& domain() {
        return *nation_instance;
    }

    template<>
    Domain<DomainType::REGION>& domain() {
        return *region_instance;
    }

    template<DomainType type>
    void attach_domain(Domain<type>& domain);

    template<DomainType type>
    void detach_domain(Domain<type>& domain);

}
