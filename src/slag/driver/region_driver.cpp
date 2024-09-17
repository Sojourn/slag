#include "region_driver.h"
#include "slag/event_loop.h"

namespace slag {

    RegionDriver::RegionDriver(EventLoop& event_loop)
        : region_(event_loop.region())
        , reactor_(event_loop.reactor())
        , region_fd_(
            make_file_descriptor(region_.file_descriptor(), FileDescriptor::Ownership::BORROWED)
        )
    {
    }

    void RegionDriver::run() {
        SLAG_PT_BEGIN();

        while (true) {
            if (!poll_) {
                poll_ = start_poll_multishot_operation(region_fd_);
            }

            SLAG_PT_WAIT_READABLE(*poll_);
            if (poll_->result() < 0) {
                poll_->cancel();
            }
            else {
                assert(poll_->result() & POLLIN); // Just curious--spurious steps are fine.

                // Our job is to wake up the event loop--done.
            }

            if (poll_->is_complete()) {
                poll_.reset();
            }
            else {
                // The operation is still active.
            }
        }

        SLAG_PT_END();
    }

}
