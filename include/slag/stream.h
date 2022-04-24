#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "stream_buffer.h"

namespace slag {
    class stream_producer;
    class stream_consumer;
    class stream_producer_transaction;
    class stream_consumer_transaction;

    class stream {
    public:
        stream(size_t minimum_capacity);
        stream(stream&&) = delete;
        stream(const stream&) = delete;
        stream& operator=(stream&&) = delete;
        stream& operator=(const stream&) = delete;

    private:
        friend class stream_producer;

        void add_producer(stream_producer& producer);
        void remove_producer(stream_producer& producer);
        void advance_producer_sequence(size_t byte_count);

    private:
        friend class stream_consumer;

        void add_consumer(stream_consumer& consumer);
        void remove_consumer(stream_consumer& consumer);
        void update_consumer_sequence();

    private:
        std::shared_ptr<stream_buffer> buffer_;
        uint64_t                       producer_sequence_;
        uint64_t                       consumer_sequence_;
        std::vector<stream_producer*>  producers_;
        std::vector<stream_consumer*>  consumers_;
    };

    class stream_producer {
    public:
        stream_producer(stream& stream);
        ~stream_producer();

        uint64_t sequence() const;
        void notify_producable();

    private:
        stream&  stream_;
        uint64_t sequence_;
    };

    class stream_consumer {
    public:
        stream_consumer(stream& s);
        ~stream_consumer();

        uint64_t sequence() const;
        void notify_consumable();

    private:
        stream&  stream_;
        uint64_t sequence_;
    };

    class stream_producer_transaction {
    public:
        stream_producer_transaction(stream_producer& producer);
        ~stream_producer_transaction();

        void rollback();
        void commit();

    private:
    }:

    class stream_consumer_transaction {
    public:
        stream_consumer_transaction(stream_consumer& consumer);
        ~stream_consumer_transaction();

        void rollback();
        void commit();

    private:
        stream_consumer*               consumer_;
        std::shared_ptr<stream_buffer> buffer_;
        uint64_t                       consumer_sequence_;
        uint64_t                       producer_sequence_;
    };
}
