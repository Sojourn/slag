#include "catch.hpp"
#include "slag/slag.h"
#include <string>

using namespace slag;

struct person {
    person() = default;
    person(person&&) = delete;
    person(const person&) = delete;
    person& operator=(person&&) = delete;
    person& operator=(const person&) = delete;

    uint32_t age = 0;
};

TEST_CASE("registry spacial locality") {
    registry<person, registry_locality::spacial> reg;

    auto&& [p1, k1] = reg.emplace();
    auto&& [p2, k2] = reg.emplace();
    auto&& [p3, k3] = reg.emplace();

    reg.erase(k2);
    reg.erase(k3);

    auto&& [p4, k4] = reg.emplace();
    CHECK(p4 == p2);
}

TEST_CASE("registry temporal locality") {
    registry<person, registry_locality::temporal> reg;

    auto&& [p1, k1] = reg.emplace();
    auto&& [p2, k2] = reg.emplace();
    auto&& [p3, k3] = reg.emplace();

    reg.erase(k2);
    reg.erase(k3);

    auto&& [p4, k4] = reg.emplace();
    CHECK(p4 == p3);
}

TEST_CASE("registry out-of-memory") {
    registry<person, registry_locality::temporal> reg(16);

    for (size_t i = 0; i < reg.capacity(); ++i) {
        reg.emplace();
    }
    CHECK(reg.size() == reg.capacity());

    auto&& [p, k] = reg.emplace();
    CHECK(p == nullptr);
    CHECK(!k);
}

TEST_CASE("registry use-after-free") {
    registry<person, registry_locality::temporal> reg;

    auto&& [p1, k1] = reg.emplace();
    CHECK(reg.find(k1));

    reg.erase(k1);

    auto&& [p2, k2] = reg.emplace();
    CHECK(p1 == p2);

    CHECK(!reg.find(k1));
    CHECK(reg.find(k2));
}
