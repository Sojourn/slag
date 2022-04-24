#pragma once

namespace slag {
    class stream_producer;
    class stream_consumer;

    class stream {
    public:
        stream(size_t minimum_capacity)
            : buffer_{std::make_shared<stream_buffer>(minimum_capacity)}
            , producer_sequence_{0}
            , consumer_sequence_{0}
            , producer_removed_{false}
            , consumer_removed_{false}
        {
        }

    private:
        friend class stream_producer;

        void add_producer(stream_producer& producer) {
            assert(std::find(producers_.begin(), producers_.end(), &producer) == producers_.end());
            assert(producer.sequence() == producer_sequence_);

            producers_.push_back(&producer);
        }
        
        void remove_producer(stream_producer& producer) {
            if (auto it = std::find(producers_.begin(), producers_.end(), &producer); it != producers_.end()) {
                producers_.erase(it);
                producer_removed_ = true;
            }

            update_producer_sequence();
        }
        
        void update_producer_sequence() {
            uint64_t minimum_producer_sequence = std::numeric_limits<decltype(producer_sequence_)>::max();
            for (auto&& producer: producers_) {
                minimum_producer_sequence = std::min(producer->sequence(), minimum_producer_sequence);
            }

            if (minimum_producer_sequence > producer_sequence_) {
                producer_sequence_ = minimum_producer_sequence;
                notify_consumers();
            }
        }

        void notify_consumers() {
            for (size_t consumer_index = 0; consumer_index < consumers_.size(); ) {
                consumer_removed_ = false;
                consumers_[consumer_index]->on_stream_consumable();
                if (!consumer_removed_) {
                    ++consumer_index;
                }
            }
        }

    private:
        std::shared_ptr<stream_buffer> buffer_;
        uint64_t                       producer_sequence_;
        uint64_t                       consumer_sequence_;
        std::vector<stream_producer*>  producers_;
        std::vector<stream_consumer*>  consumers_;
        bool                           consumer_removed_;
        bool                           producer_removed_;
    };

    class stream_consumer {
    public:
        stream_consumer(stream& s);

    private:
        stream&  stream_;
        uint64_t sequence_;
    };
}