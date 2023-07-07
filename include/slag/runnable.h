#pragma once

namespace slag {

    class Runnable {
    public:
        virtual ~Runnable() = default;

        virtual void run() = 0;
    };

}
