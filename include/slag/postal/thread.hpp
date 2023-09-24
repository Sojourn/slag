#include <stdexcept>
#include <cassert>

namespace slag {

    template<typename Driver>
    template<typename... DriverArgs>
    inline Thread<Driver>::Thread(const RegionConfig& config, DriverArgs&&... driver_args) {
        std::promise<void> started_promise;
        std::future<void> started_future = started_promise.get_future();

        thread_ = std::thread([&]() {
            std::optional<Region> region_storage;
            std::optional<Driver> driver_storage;

            // Setup the region/driver.
            {
                try {
                    region_storage.emplace(config);
                    driver_storage.emplace(std::forward<DriverArgs>(driver_args)...);
                }
                catch (const std::exception&) {
                    started_promise.set_exception(std::current_exception());
                    return;
                }

                // Wake the spawning thread now that we have initialized the region/driver,
                // and no longer need to use either the config or driver_arg references.
                started_promise.set_value();
            }

            // Run the driver until completion.
            {
                try {
                    run(*driver_storage);
                }
                catch (const std::exception&) {
                    complete_promise_.set_exception(std::current_exception());
                    return;
                }

                complete_promise_.set_value();
            }
        });

        try {
            started_future.get();
        }
        catch (const std::exception&) {
            thread_.join();
            throw;
        }
    }

    template<typename Driver>
    inline Thread<Driver>::~Thread() {
        thread_.join();
    }

    template<typename Driver>
    inline std::future<void> Thread<Driver>::get_future() {
        return complete_promise_.get_future();
    }

    template<typename Driver>
    inline void Thread<Driver>::run(Driver& driver) {
        driver.set_state(TaskState::RUNNING);

        // The driver has no way of waiting, and is expected to do so internally.
        while (driver.is_running()) {
            driver.run();
        }
    }

}
