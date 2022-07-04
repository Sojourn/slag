#pragma once

#include <liburing.h>        
#include "reactor.h"

namespace slag {

    class IOURingReactor : public Reactor {
    public:
        IOURingReactor();
        ~IOURingReactor();

    private:
        void startup() override;
        void step() override;
        void shutdown() override;

    private:
    };

}
