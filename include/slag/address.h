#pragma once

#include "slag/platform.h"

namespace slag {

    class Address {
    public:
        Address();
        explicit Address(const struct sockaddr& address, socklen_t size);
        explicit Address(const struct sockaddr_in& address);
        explicit Address(const struct sockaddr_in6& address);
        Address(const Address& other);
        ~Address() = default;

        Address& operator=(const Address& rhs);

        [[nodiscard]] sa_family_t family() const;
        [[nodiscard]] socklen_t size() const;
        [[nodiscard]] struct sockaddr& addr();
        [[nodiscard]] const struct sockaddr& addr() const;
        [[nodiscard]] struct sockaddr_in& addr_in();
        [[nodiscard]] const struct sockaddr_in& addr_in() const;
        [[nodiscard]] struct sockaddr_in6& addr_in6();
        [[nodiscard]] const struct sockaddr_in6& addr_in6() const;

    private:
        struct sockaddr_storage storage_;
    };

}
