#include "slag/address.h"
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

namespace slag {

    Address::Address() {
        memset(this, 0, sizeof(*this));
    }

    Address::Address(const struct sockaddr& address, socklen_t size) {
        assert(size <= sizeof(storage_));
        memcpy(&storage_, &address, size);
    }

    Address::Address(const struct sockaddr_in& address)
        : Address(reinterpret_cast<const struct sockaddr&>(address), sizeof(address))
    {
    }

    Address::Address(const struct sockaddr_in6& address)
        : Address(reinterpret_cast<const struct sockaddr&>(address), sizeof(address))
    {
    }

    Address::Address(const Address& other) {
        operator=(other);
    }

    Address& Address::operator=(const Address& rhs) {
        if (this != &rhs) {
            memcpy(&storage_, &rhs.storage_, rhs.size());
        }

        return *this;
    }

    sa_family_t Address::family() const {
        return storage_.ss_family;
    }

    socklen_t Address::size() const {
        switch (family()) {
            case AF_INET: {
                return sizeof(struct sockaddr_in);
            }
            case AF_INET6: {
                return sizeof(struct sockaddr_in6);
            }
            default: {
                return sizeof(storage_);
            }
        }
    }

    struct sockaddr& Address::addr() {
        return reinterpret_cast<struct sockaddr&>(storage_);
    }

    const struct sockaddr& Address::addr() const {
        return reinterpret_cast<const struct sockaddr&>(storage_);
    }

    struct sockaddr_in& Address::addr_in() {
        assert(family() == AF_INET);
        return reinterpret_cast<struct sockaddr_in&>(storage_);
    }

    const struct sockaddr_in& Address::addr_in() const {
        assert(family() == AF_INET);
        return reinterpret_cast<const struct sockaddr_in&>(storage_);
    }

    struct sockaddr_in6& Address::addr_in6() {
        assert(family() == AF_INET6);
        return reinterpret_cast<struct sockaddr_in6&>(storage_);
    }

    const struct sockaddr_in6& Address::addr_in6() const {
        assert(family() == AF_INET6);
        return reinterpret_cast<const struct sockaddr_in6&>(storage_);
    }

    std::string Address::to_pretty_string() const {
        static constexpr size_t buffer_length = std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN);
        char buffer[buffer_length];
        uint16_t port = 0;

        switch (family()) {
            case AF_INET: {
                if (!inet_ntop(AF_INET, &addr_in().sin_addr, buffer, static_cast<socklen_t>(buffer_length))) {
                    make_system_error().raise("Failed to convert address to pretty string");
                }
                port = ntohs(addr_in().sin_port);
                break;
            }
            case AF_INET6: {
                if (!inet_ntop(AF_INET6, &addr_in6().sin6_addr, buffer, static_cast<socklen_t>(buffer_length))) {
                    make_system_error().raise("Failed to convert address to pretty string");
                }
                port = ntohs(addr_in6().sin6_port);
                break;
            }
            case AF_UNSPEC: {
                make_system_error(-EAFNOSUPPORT).raise("Failed to convert address to pretty string");
                break;
            }
        }

        return fmt::format("{}*{}", buffer, port);
    }

    std::vector<Address> execute(const AddressQuery& query) {
        const char* node = query.host_name.c_str();
        const char* service = nullptr;

        std::string service_storage;
        if (auto service_name = std::get_if<AddressQuery::ServiceName>(&query.service)) {
            service = service_name->c_str();
        }
        else if (auto service_port = std::get_if<AddressQuery::ServicePort>(&query.service)) {
            service_storage = fmt::format("{}", *service_port);
            service = service_storage.c_str();
        }

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));

        hints.ai_flags |= (AI_V4MAPPED | AI_ADDRCONFIG);
        if (query.passive) {
            hints.ai_flags |= AI_PASSIVE;
        }
        if (std::holds_alternative<AddressQuery::ServicePort>(query.service)) {
            hints.ai_flags |= AI_NUMERICSERV;
        }

        hints.ai_family   = query.family;
        hints.ai_socktype = query.type;
        hints.ai_protocol = query.protocol;

        struct addrinfo* records = nullptr;
        int error_code = 0;
        do {
            error_code = ::getaddrinfo(node, service, &hints, &records);
            if (error_code && error_code != EAI_SYSTEM) {
                throw std::runtime_error(gai_strerror(error_code));
            }
        } while (error_code && (errno == EAGAIN));

        if (error_code) {
            assert(error_code == EAI_SYSTEM);
            throw std::runtime_error(strerror(errno));
        }

        std::vector<Address> result;
        for (struct addrinfo* record = records; record; record = record->ai_next) {
            result.emplace_back(*record->ai_addr, record->ai_addrlen);
        }

        // FIXME: do this w/ RAII
        freeaddrinfo(records);

        return result;
    }

}
