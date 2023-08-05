#pragma once

#define SLAG_POSTAL_DOMAIN_TYPES(X) \
    X(EMPIRE)                       \
    X(NATION)                       \
    X(REGION)                       \

namespace slag::postal {

    enum class DomainType {
#define X(SLAG_POSTAL_DOMAIN_TYPE) SLAG_POSTAL_DOMAIN_TYPE,
        SLAG_POSTAL_DOMAIN_TYPES(X)

#undef X
    };

    template<DomainType type>
    class Domain;

    template<DomainType type>
    Domain<type>& domain();

    template<DomainType type>
    void attach_domain(Domain<type>& domain);

    template<DomainType type>
    void detach_domain(Domain<type>& domain);

    template<>
    class Domain<DomainType::EMPIRE> {
        Domain(Domain&&) = delete;
        Domain(const Domain&) = delete;
        Domain& operator=(Domain&&) = delete;
        Domain& operator=(const Domain&) = delete;

    public:
        Domain() {
            attach_domain(*this);
        }

        ~Domain() {
            detach_domain(*this);
        }
    };

    template<>
    class Domain<DomainType::NATION> {
        Domain(Domain&&) = delete;
        Domain(const Domain&) = delete;
        Domain& operator=(Domain&&) = delete;
        Domain& operator=(const Domain&) = delete;

    public:
        Domain() {
            attach_domain(*this);
        }

        ~Domain() {
            detach_domain(*this);
        }
    };

    template<>
    class Domain<DomainType::REGION> {
        Domain(Domain&&) = delete;
        Domain(const Domain&) = delete;
        Domain& operator=(Domain&&) = delete;
        Domain& operator=(const Domain&) = delete;

    public:
        Domain() {
            attach_domain(*this);
        }

        ~Domain() {
            detach_domain(*this);
        }
    };

}
