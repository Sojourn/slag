#include <stdexcept>
#include <cassert>

namespace slag {

    template<OperationType type>
    inline OperationHandle<type>::OperationHandle(Operation<type>& operation)
        : operation_{&operation}
    {
    }

    template<OperationType type>
    inline OperationHandle<type>::OperationHandle()
        : operation_{nullptr}
    {
    }

    template<OperationType type>
    inline OperationHandle<type>::OperationHandle(OperationHandle&& other)
        : OperationHandle{}
    {
        std::swap(operation_, other.operation_);
    }

    template<OperationType type>
    inline OperationHandle<type>::~OperationHandle() {
        reset();
    }

    template<OperationType type>
    inline OperationHandle<type>& OperationHandle<type>::operator=(OperationHandle&& that) {
        if (this != &that) {
            reset();

            std::swap(operation_, that.operation_);
        }

        return *this;
    }

    template<OperationType type>
    inline OperationHandle<type>::operator bool() const {
        return static_cast<bool>(operation_);
    }

    template<OperationType type>
    inline Operation<type>& OperationHandle<type>::operator*() {
        assert(operation_);
        return *operation_;
    }

    template<OperationType type>
    inline const Operation<type>& OperationHandle<type>::operator*() const {
        assert(operation_);
        return *operation_;
    }

    template<OperationType type>
    inline Operation<type>* OperationHandle<type>::operator->() {
        assert(operation_);
        return operation_;
    }

    template<OperationType type>
    inline const Operation<type>* OperationHandle<type>::operator->() const {
        assert(operation_);
        return operation_;
    }

    template<OperationType type>
    inline void OperationHandle<type>::reset() {
        if (operation_) {
            operation_->abandon();
            operation_ = nullptr;
        }

        assert(!operation_);
    }

}
