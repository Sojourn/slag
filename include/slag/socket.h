#pragma once

#include "slag/address.h"
#include "slag/future.h"
#include "slag/resource.h"
#include "slag/coroutine.h"
#include "slag/operation.h"
#include "slag/file_descriptor.h"

namespace slag {

    class Socket : public Resource {
    public:
        Coroutine<Result<void>> open(sa_family_t family, int type, int protocol=0);

    private:
        void handle_operation_complete(Operation& operation) override;
    };

}
