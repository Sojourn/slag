#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

namespace slag::postal {

    class Empire;
    class Nation;
    class Region;

    enum class DomainType {
        EMPIRE,
        NATION,
        REGION,
    };

    Empire& empire();
    Nation& nation();
    Region& region();

    class Domain {
        Domain(Domain&&) = delete;
        Domain(const Domain&) = delete;
        Domain& operator=(Domain&&) = delete;
        Domain& operator=(const Domain&) = delete;

    protected:
        Domain(DomainType type, uint16_t identity);
        ~Domain();

    public:
        DomainType type() const;
        uint16_t identity() const;

    private:
        DomainType type_;
        uint16_t   identity_;
    };

    class Empire : public Domain {
    public:
        explicit Empire(uint16_t identity);
    };

    class Nation : public Domain {
    public:
        explicit Nation(uint16_t identity);

    private:
        Empire& empire_;
    };

    class Region : public Domain {
    public:
        explicit Region(uint16_t identity);

    private:
        Nation& nation_;
    };

}
