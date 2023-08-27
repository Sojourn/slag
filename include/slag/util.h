#pragma once

#include <tuple>
#include <utility>
#include <concepts>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace slag {

    static constexpr size_t DEFAULT_ALIGNMENT = alignof(void*);

    // size_bytes_v?
    template<typename T>
    constexpr inline size_t SIZE_BYTES = sizeof(T) / sizeof(std::byte);

    // size_bits_v?
    template<typename T>
    constexpr inline size_t SIZE_BITS = SIZE_BYTES<T> * 8;

    // TODO: remove once the standard library includes an integer version of this
    inline uint64_t log2(uint64_t value) {
        return 64 - __builtin_clzll(value) - 1;
    }

    constexpr inline void set_bit(uint64_t& mask, size_t index) {
        mask |= (1ull << index);
    }

    constexpr inline void reset_bit(uint64_t& mask, size_t index) {
        mask &= ~(1ull << index);
    }

    constexpr inline bool test_bit(uint64_t& mask, size_t index) {
        return (mask & (1ull << index)) != 0;
    }

    template<typename T>
    constexpr inline bool is_overlapping(const std::pair<T, T>& a, const std::pair<T, T>& b) {
        return std::max(a.first, b.first) < std::min(a.second, b.second);
    }

    template<size_t ALIGNMENT = DEFAULT_ALIGNMENT>
    inline uintptr_t align_forward(uintptr_t address) {
        return (address + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
    }

    template<typename T, size_t ALIGNMENT = DEFAULT_ALIGNMENT>
    inline T* align_forward(T* pointer) {
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
        uintptr_t aligned_address = align_forward<ALIGNMENT>(address);
        return reinterpret_cast<T*>(aligned_address);
    }

    template<typename T>
    struct TypeWrapper {
        using type = T;
    };

    template<size_t I>
    struct IndexWrapper {
        static constexpr size_t value = I;
    };

    template<int I, typename TargetType, typename... Types>
    struct FindType;

    template<int I, typename TargetType>
    struct FindType<I, TargetType> {
        static constexpr int value = -1; // base case
    };

    template<int I, typename TargetType, typename Type, typename... Types>
    struct FindType<I, TargetType, Type, Types...> {
        static constexpr int value = (
            std::is_same_v<TargetType, Type> ? I : FindType<I+1, TargetType, Types...>::value
        );
    };

    template<typename TargetType, typename... Types>
    constexpr int find_type_v = FindType<0, TargetType, Types...>::value;

    template<typename T, typename IndexSequence>
    struct tuple_reverse_impl;

    template<typename T, size_t... I>
    struct tuple_reverse_impl<T, std::index_sequence<I...>> {
        using type = std::tuple<typename std::tuple_element<sizeof...(I) - 1 - I, T>::type...>;
    };

    // handle empty tuples
    template<typename T>
    struct tuple_reverse_impl<T, std::index_sequence<>> {
        using type = T;
    };

    template<typename T>
    using tuple_reverse_t = tuple_reverse_impl<T, std::make_index_sequence<std::tuple_size<T>::value>>::type;

    // raise a system error for the currently set errno
    void raise_system_error(const char* message);

    // set the cpu affinity of the current thread
    void set_cpu_affinity(int cpu_affinity);

}
