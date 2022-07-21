#include "slag/address.h"
#include <cstdlib>
#include <cstring>
#include <cassert>

slag::Address::Address() {
    memset(this, 0, sizeof(*this));
}

slag::Address::Address(const struct sockaddr& address, socklen_t size) {
    assert(size <= sizeof(storage_));
    memcpy(&storage_, &address, size);
}

slag::Address::Address(const struct sockaddr_in& address)
    : Address(reinterpret_cast<const struct sockaddr&>(address), sizeof(address))
{
}

slag::Address::Address(const struct sockaddr_in6& address)
    : Address(reinterpret_cast<const struct sockaddr&>(address), sizeof(address))
{
}

slag::Address::Address(const Address& other) {
    operator=(other);
}

slag::Address& slag::Address::operator=(const Address& rhs) {
    if (this != &rhs) {
        memcpy(&storage_, &rhs.storage_, rhs.size());
    }

    return *this;
}

sa_family_t slag::Address::family() const {
    return storage_.ss_family;
}

socklen_t slag::Address::size() const {
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

struct sockaddr& slag::Address::addr() {
    assert(family());
    return reinterpret_cast<struct sockaddr&>(storage_);
}

const struct sockaddr& slag::Address::addr() const {
    assert(family());
    return reinterpret_cast<const struct sockaddr&>(storage_);
}

struct sockaddr_in& slag::Address::addr_in() {
    assert(family() == AF_INET);
    return reinterpret_cast<struct sockaddr_in&>(storage_);
}

const struct sockaddr_in& slag::Address::addr_in() const {
    assert(family() == AF_INET);
    return reinterpret_cast<const struct sockaddr_in&>(storage_);
}

struct sockaddr_in6& slag::Address::addr_in6() {
    assert(family() == AF_INET6);
    return reinterpret_cast<struct sockaddr_in6&>(storage_);
}

const struct sockaddr_in6& slag::Address::addr_in6() const {
    assert(family() == AF_INET6);
    return reinterpret_cast<const struct sockaddr_in6&>(storage_);
}
