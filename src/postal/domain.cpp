#include "slag/postal/domain.h"
#include <cassert>

namespace slag::postal {

    static Domain* empire_instance = nullptr;
    static Domain* nation_instance = nullptr;
    thread_local Domain* region_instance = nullptr;

    // Returns a pointer to the instance pointer of this type.
    Domain** domain_instance_pointer(DomainType type) {
        switch (type) {
            case DomainType::EMPIRE: return &empire_instance;
            case DomainType::NATION: return &nation_instance;
            case DomainType::REGION: return &region_instance;
        }

        abort();
    }

    Empire& empire() {
        assert(empire_instance);
        return static_cast<Empire&>(*empire_instance);
    }

    Nation& nation() {
        assert(nation_instance);
        return static_cast<Nation&>(*nation_instance);
    }

    Region& region() {
        assert(region_instance);
        return static_cast<Region&>(*region_instance);
    }

    Domain::Domain(DomainType type, uint16_t identity)
        : type_{type}
        , identity_{identity}
    {
        Domain** instance = domain_instance_pointer(type_);
        assert(!*instance);
        *instance = this;
    }

    Domain::~Domain() {
        Domain** instance = domain_instance_pointer(type_);
        assert(*instance == this);
        *instance = nullptr;
    }

    DomainType Domain::type() const {
        return type_;
    }

    uint16_t Domain::identity() const {
        return identity_;
    }

    Empire::Empire(uint16_t identity)
        : Domain{DomainType::EMPIRE, identity}
    {
    }

    Nation::Nation(uint16_t identity)
        : Domain{DomainType::NATION, identity}
        , empire_{empire()}
    {
    }

    Region::Region(uint16_t identity)
        : Domain{DomainType::REGION, identity}
        , nation_{nation()}
    {
    }

}
