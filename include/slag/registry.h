#pragma once

#include <sys/mman.h>
#include <new>
#include <optional>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cassert>

namespace slag {
    static constexpr size_t registry_index_bits = 24;
    static constexpr size_t registry_nonce_bits = 32;

    enum class registry_locality {
        spacial,
        temporal,
    };

    struct registry_key_base {
        uintptr_t index : registry_index_bits;
        uintptr_t nonce : registry_nonce_bits;
        uintptr_t alive : 1;
    };
    static_assert(sizeof(registry_key_base) <= sizeof(uintptr_t));

    template<typename T>
    struct registry_key : registry_key_base {
        registry_key() {
            memset(this, 0, sizeof(*this));
        }

        explicit operator bool() const {
            return alive;
        }

        uintptr_t encode() const {
            uintptr_t result = 0;
            memcpy(&result, &static_cast<const registry_key_base&>(*this), sizeof(registry_key_base));
            return result;
        }

        static registry_key decode(uintptr_t encoded_value) {
            registry_key result;
            memcpy(&static_cast<registry_key_base&>(result), &encoded_value, sizeof(registry_key_base));
            return result;
        }
    };

    template<typename T, registry_locality locality>
    class registry {
    public:
        using key = registry_key<T>;
        static constexpr size_t max_capacity = 1ull << registry_index_bits;

        registry(size_t capacity=max_capacity)
            : storage_base_(nullptr)
            , storage_size_(capacity * sizeof(T))
            , capacity_(capacity)
        {
            void* storage_base = mmap(nullptr, storage_size_, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
            if (storage_base == MAP_FAILED) {
                throw std::runtime_error(strerror(errno));
            }

            storage_base_ = static_cast<uint8_t*>(storage_base);
        }

        ~registry() {
            clear();

            int err = munmap(storage_base_, storage_size_);
            assert(!err);
        }

        size_t capacity() const {
            return capacity_;
        }

        size_t size() const {
            return size_;
        }

        bool empty() const {
            return size_ == 0;
        }

        template<typename... Args>
        std::pair<T*, key> emplace(Args&&... args) {
            if (size_ == capacity_) {
                return std::make_pair(nullptr, key{});
            }

            // allocate storage for the value
            key k;
            if (free_slots_.empty()) {
                k.index = nonces_.size();
                nonces_.push_back(0);
            }
            else {
                if constexpr (locality == registry_locality::spacial) {
                    std::pop_heap(free_slots_.begin(), free_slots_.end(), std::greater<>{});
                }
                k.index = free_slots_.back();
                free_slots_.pop_back();
            }

            // initialize the value
            T* value = nullptr;
            try {
                value = new(&get_value(k.index)) T(std::forward<Args>(args)...);
            }
            catch (const std::exception&) {
                free_slots_.push_back(k.index);
                if constexpr (locality == registry_locality::spacial) {
                    std::push_heap(free_slots_.begin(), free_slots_.end(), std::greater<>{});
                }
                throw;
            }

            size_ += 1;
            k.nonce = nonces_[k.index];
            k.alive = true;
            return std::make_pair(value, k);
        }

        T* find(key k) {
            if (!is_valid_key()) {
                return nullptr;
            }

            return &get_value(k.index);
        }

        const T* find(key k) const {
            if (!is_valid_key()) {
                return nullptr;
            }

            return &get_value(k.index);
        }

        void erase(key k) {
            if (!is_valid_key(k)) {
                return;
            }

            get_value(k.index).~T();
            size_ -= 1;
            nonces_[k.index] += 1;
            free_slots_.push_back(k.index);
            if constexpr (locality == registry_locality::spacial) {
                std::push_heap(free_slots_.begin(), free_slots_.end(), std::greater<>{});
            }
        }

        void clear() {
            std::sort(free_slots_.begin(), free_slots_.end());
            size_t free_slot_off = 0;
            size_t free_slot_cnt = free_slots_.size();

            for (uint32_t index = 0; index < static_cast<uint32_t>(nonces_.size()); ++index) {
                if ((free_slot_off < free_slot_off) && (free_slots_[free_slot_off] == index)) {
                    ++free_slot_off;
                    continue;
                }

                get_value(index).~T();
                nonces_[index] += 1;
                free_slots_.push_back(index);
            }

            if constexpr (locality == registry_locality::spacial) {
                std::make_heap(free_slots_.begin(), free_slots_.end(), std::greater<>{});
            }

            size_ = 0;
        }

    private:
        bool is_valid_key(key k) const {
            return k && (k.nonce == nonces_[k.index]);
        }

        T& get_value(uint32_t index) {
            size_t offset = index * sizeof(T);
            assert(offset < storage_size_);
            return reinterpret_cast<T&>(storage_base_[offset]);
        }

        const T& get_value(uint32_t index) const {
            size_t offset = index * sizeof(T);
            assert(offset < storage_size_);
            return reinterpret_cast<const T&>(storage_base_[offset]);
        }

    private:
        uint8_t*              storage_base_;
        size_t                storage_size_;
        size_t                capacity_;
        size_t                size_;
        std::vector<uint32_t> nonces_;
        std::vector<uint32_t> free_slots_;
    };

}
