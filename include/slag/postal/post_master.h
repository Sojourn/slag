#pragma once

namespace slag {

    // TODO: rename this to PostMaster
    class PostMaster : public Task {
    public:
        PostMaster(Region& region);

    private:
        // TODO: This loop structure needs some refinement. Also think about splitting import/process/export phases.
        void run() override {
            std::array<Parcel*, 64> parcels;

            for (auto&& queue: region_.imports()) {
                while (size_t parcel_count = queue.poll(parcels)) {
                    for (size_t i = 0; i < parcel_count; ++i) {
                        Parcel* parcel = parcels[i];
                        assert(parcel);

                        if (PostBox* post_box = post_office_.post_box(parcel.to)) {
                            post_box->incoming().push_back(*parcel);
                        }
                        else {
                            // discard the parcel?
                        }
                    }
                }

                queue.flush();
            }

            for (auto&& queue: region_.exports()) {
                // TODO: work on the polling system
            }

            schedule();
        }

    private:
        Region&     region_;
        PostOffice& post_office_;
    };

}
