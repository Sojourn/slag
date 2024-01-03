#include <new>
#include <utility>
#include <cassert>

namespace slag {

    template<typename T>
    inline Queue<T>::Queue()
        : capacity_{0}
        , mask_{0}
        , head_{0}
        , tail_{0}
    {
    }

    template<typename T>
    inline Queue<T>::~Queue() {
        clear();
    }

    template<typename T>
    inline Queue<T>::Queue(Queue&& other)
        : capacity_{other.capacity_}
        , mask_{other.mask_}
        , head_{other.head_}
        , tail_{other.tail_}
        , slots_{std::move(other.slots_)}
    {
        other.capacity_ = 0;
        other.mask_     = 0;
        other.head_     = 0;
        other.tail_     = 0;
    }

    template<typename T>
    inline Queue<T>& Queue<T>::operator=(Queue&& rhs) {
        if (this != &rhs) {
            clear();

            capacity_       = rhs.capacity_;
            mask_           = rhs.mask_;
            head_           = rhs.head_;
            tail_           = rhs.tail_;
            slots_          = std::move(rhs.slots_);

            rhs.capacity_ = 0;
            rhs.mask_     = 0;
            rhs.head_     = 0;
            rhs.tail_     = 0;
        }

        return *this;
    }

    template<typename T>
    inline bool Queue<T>::is_empty() const {
        return head_ == tail_;
    }

    template<typename T>
    inline size_t Queue<T>::size() const {
        return tail_ - head_;
    }

    template<typename T>
    inline size_t Queue<T>::capacity() const {
        return capacity_;
    }

    template<typename T>
    inline T& Queue<T>::front() {
        assert(!is_empty());
        return operator[](0);
    }

    template<typename T>
    inline const T& Queue<T>::front() const {
        assert(!is_empty());
        return operator[](0);
    }

    template<typename T>
    inline T& Queue<T>::back() {
        assert(!is_empty());
        return operator[](size() - 1);
    }

    template<typename T>
    inline const T& Queue<T>::back() const {
        assert(!is_empty());
        return operator[](size() - 1);
    }

    template<typename T>
    inline T& Queue<T>::operator[](size_t index) {
        assert(index < size());

        Slot& slot = slots_[mask_& (head_ + index)];
        return slot.get();
    }

    template<typename T>
    inline const T& Queue<T>::operator[](size_t index) const {
        assert(index < size());

        const Slot& slot = slots_[mask_& (head_ + index)];
        return slot.get();
    }

    template<typename T>
    template<typename... Args>
    inline void Queue<T>::emplace_back(Args&&... args) {
        if ((tail_ - head_) == capacity_) {
            size_t new_capacity = std::max(capacity_ * 2, static_cast<size_t>(8));
            reserve(new_capacity);
        }

        Slot& slot = slots_[mask_ & tail_++];
        slot.create(std::forward<Args>(args)...);
    }

    template<typename T>
    inline void Queue<T>::push_back(T value) {
        emplace_back(std::move(value));
    }

    template<typename T>
    inline std::optional<T> Queue<T>::pop_front() {
        if (is_empty()) {
            return std::nullopt;
        }


        Slot& slot = slots_[mask_ & head_++];
        std::optional<T> result(std::move(slot.get()));
        slot.destroy();

        return result;
    }

    template<typename T>
    inline bool Queue<T>::consume_front(size_t count) {
        if (size() < count) {
            return false;
        }

        for (size_t i = 0; i < count; ++i) {
            Slot& slot = slots_[mask_ & (head_ + i)];
            slot.destroy();
        }

        head_ += count;

        return true;
    }

    template<typename T>
    inline void Queue<T>::clear() {
        bool ok = consume_front(size());
        assert(ok);
    }

    template<typename T>
    inline void Queue<T>::reserve(size_t minimum_capacity) {
        if (minimum_capacity <= capacity_) {
            return; // already at or above the requested capacity
        }

        size_t new_capacity = 1;
        while (new_capacity < minimum_capacity) {
            new_capacity *= 2;
        }

        size_t new_mask = new_capacity - 1;
        std::unique_ptr<Slot[]> new_slots(new Slot[new_capacity]);

        if (slots_) {
            for (size_t sequence = head_; sequence != tail_; ++sequence) {
                auto&& old_slot = slots_[sequence & mask_];
                auto&& new_slot = new_slots[sequence & new_mask];

                // extract the element from the old slot, and destroy it
                new_slot.create(std::move(old_slot.get()));
                old_slot.destroy();
            }
        }
        else {
            assert(capacity_ == 0);
        }

        capacity_ = new_capacity;
        mask_ = new_mask;
        slots_ = std::move(new_slots);
    }

    template<typename T>
    template<typename... Args>
    inline void Queue<T>::Slot::create(Args&&... args) {
        new(storage_.data()) T(std::forward<Args>(args)...);
    }

    template<typename T>
    inline void Queue<T>::Slot::destroy() {
        get().~T();
    }

    template<typename T>
    inline T& Queue<T>::Slot::get() {
        return *reinterpret_cast<T*>(storage_.data());
    }

    template<typename T>
    inline const T& Queue<T>::Slot::get() const {
        return *reinterpret_cast<const T*>(storage_.data());
    }

}
