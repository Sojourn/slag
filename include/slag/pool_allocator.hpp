#include <new>
#include <tuple>

template<typename T>
slag::PoolAllocator<T>::PoolAllocator(size_t initial_capacity) {
    while (unused_storage_.size() < initial_capacity) {
        allocate_block();
    }
}

template<typename T>
slag::PoolAllocator<T>::~PoolAllocator() {
    assert(unused_storage_.size() == (storage_blocks_.size() * STORAGE_BLOCK_SIZE_));
}

template<typename T>
template<typename... Args>
T& slag::PoolAllocator<T>::allocate(Args&&... args) {
    if (unused_storage_.empty()) {
        allocate_block();
    }

    Storage* storage = unused_storage_.back();
    unused_storage_.pop_back();
    return *(new(storage) T{std::forward<Args>(args)...});
}

template<typename T>
void slag::PoolAllocator<T>::deallocate(T& object) {
    Storage& storage = reinterpret_cast<Storage&>(object);
    object.~T();
    unused_storage_.push_back(&storage);
}

template<typename T>
void slag::PoolAllocator<T>::allocate_block() {
    storage_blocks_.push_back(std::make_unique<StorageBlock>());
    for (Storage& storage: *storage_blocks_.back()) {
        unused_storage_.push_back(&storage);
    }
}
