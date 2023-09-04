#include <stdexcept>
#include <cassert>

namespace slag::postal {

    template<OperationType type>
    OperationHandle<type>::OperationHandle(Operation<type>& operation)
        : operation_{&operation}
    {
    }

    template<OperationType type>
    OperationHandle<type>::OperationHandle()
        : operation_{nullptr}
    {
    }

    template<OperationType type>
    OperationHandle<type>::OperationHandle(OperationHandle&& other)
        : OperationHandle{}
    {
        std::swap(operation_, other.operation_);
    }

    template<OperationType type>
    OperationHandle<type>::~OperationHandle() {
        reset();
    }

    template<OperationType type>
    OperationHandle<type>& OperationHandle<type>::operator=(OperationHandle&& that) {
        if (this != &that) {
            reset();

            std::swap(operation_, that.operation_);
        }

        return *this;
    }

    template<OperationType type>
    OperationHandle<type>::operator bool() const {
        return static_cast<bool>(operation_);
    }

    template<OperationType type>
    Operation<type>& OperationHandle<type>::operator*() {
        assert(operation_);
        return *operation_;
    }

    template<OperationType type>
    const Operation<type>& OperationHandle<type>::operator*() const {
        assert(operation_);
        return *operation_;
    }

    template<OperationType type>
    Operation<type>* OperationHandle<type>::operator->() {
        assert(operation_);
        return operation_;
    }

    template<OperationType type>
    const Operation<type>* OperationHandle<type>::operator->() const {
        assert(operation_);
        return operation_;
    }

    template<OperationType type>
    void OperationHandle<type>::reset() {
        if (operation_) {
            operation_->abandon();
            operation_ = nullptr;
        }

        assert(!operation_);
    }

}
