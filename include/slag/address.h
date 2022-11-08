#pragma once

#include <variant>
#include <string>
#include <vector>
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

        [[nodiscard]] std::string to_pretty_string() const;

    private:
        struct sockaddr_storage storage_;
    };

    struct AddressQuery {
        using ServiceName = std::string;
        using ServicePort = int;
        using Service     = std::variant<std::monostate, ServiceName, ServicePort>;

        std::string host_name;
        Service     service;
        int         family   = AF_UNSPEC;
        int         type     = 0;
        int         protocol = 0;
        bool        passive  = false;
    };

    std::vector<Address> execute(const AddressQuery& query);

}
