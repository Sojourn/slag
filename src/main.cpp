#include <iostream>
#include <string>

#include "mantle/mantle.h"
#include "slag/slag.h"

using namespace slag;

class Worker : public ProtoTask {
public:
    Worker()
        : channel_(std::to_string(get_thread().index()))
    {
        std::cout << (int)get_thread().index() << " constructed" << std::endl;
    }

    void run() override final {
        try {
            SLAG_PT_BEGIN();
            {
                target_ = channel_.query("0");
                assert(target_);

                std::cout << (int)get_thread().index() << " sending message" << std::endl;
                channel_.send(*target_, bind(new Message));

                timer_.set(std::chrono::milliseconds(1000));
                looping_ = true;
                while (looping_) {
                    if (timer_.is_expired()) {
                        looping_ = false;
                    }
                    if (Ptr<Message> message = channel_.receive()) {
                        std::cout << get_thread().index() << " received message" << std::endl;
                    }

                    SLAG_PT_YIELD();
                }
            }

            asm("int $3");
            SLAG_PT_END();
        }
        catch (const std::exception& ex) {
            std::cout << (int)get_thread().index() << " exception " << ex.what() << std::endl;
        }
    }

private:
    Channel channel_;
    std::optional<ChannelId> target_;
    BasicTimer timer_;
    bool looping_ = false;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    try {
        Runtime runtime;
        for (size_t i = 0; i < 4; ++i) {
            runtime.spawn_thread<Worker>(ThreadConfig {
                .name           = "worker",
                .cpu_affinities = std::nullopt,
            });
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Caught: " << ex.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
