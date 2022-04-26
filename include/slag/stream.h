#pragma once

#include <span>
#include <optional>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include "stream_buffer.h"

namespace slag {
    class stream;
    class stream_producer;
    class stream_consumer;
    class stream_transaction_aborted;
    class stream_producer_transaction;
    class stream_consumer_transaction;

    using stream_sequence = uint32_t;

    enum class stream_event {
        DATA_PRODUCED = (1 << 0),
        DATA_CONSUMED = (1 << 1),
    };

    stream_event operator&(stream_event l, stream_event r);
    void operator&=(stream_event& l, stream_event r);
    stream_event operator|(stream_event l, stream_event r);
    void operator|=(stream_event& l, stream_event r);
    stream_event operator^(stream_event l, stream_event r);
    void operator^=(stream_event& l, stream_event r);

    class stream_transaction_aborted : public std::exception {
    public:
        using std::exception::exception;

        const char* what() const noexcept override;
    };

    class stream {
        stream(stream&&) = delete;
        stream(const stream&) = delete;
        stream& operator=(stream&&) = delete;
        stream& operator=(const stream&) = delete;

    public:
        stream(size_t minimum_capacity);
        ~stream();

    private:
        friend class stream_producer;

        size_t producer_segment_size() const;
        std::span<std::byte> producer_segment();
        void resize_producer_segment(size_t minimum_size);
        void advance_producer_sequence(size_t byte_count);

        void add_producer(stream_producer& producer);
        void remove_producer(stream_producer& producer);
        size_t active_producer_transaction_count() const;

    private:
        friend class stream_consumer;

        size_t consumer_segment_size() const;
        std::span<const std::byte> consumer_segment() const;
        void update_consumer_sequence();

        void add_consumer(stream_consumer& consumer);
        void remove_consumer(stream_consumer& consumer);
        size_t active_consumer_transaction_count() const;
    
    private:
        friend class stream_observer;

        void add_observer(stream_observer& observer);
        void remove_observer(stream_observer& observer);
        void notify_observers(stream_event event);

    private:
        std::shared_ptr<stream_buffer> buffer_;
        stream_sequence                producer_sequence_;
        stream_sequence                consumer_sequence_;
        std::vector<stream_producer*>  producers_;
        std::vector<stream_consumer*>  consumers_;
        std::vector<stream_observer*>  producer_observers_;
        std::vector<stream_observer*>  consumer_observers_;
    };

    class stream_producer_transaction {
        friend class stream_producer;

        stream_producer_transaction(stream_producer& producer);

    public:
        stream_producer_transaction(stream_producer_transaction&& other);
        stream_producer_transaction(const stream_producer_transaction& other) = delete;
        stream_producer_transaction& operator=(stream_producer_transaction&& other);
        stream_producer_transaction& operator=(const stream_producer_transaction& other) = delete;
        ~stream_producer_transaction();

        bool is_aborted() const;
        explicit operator bool() const;

        void rollback();
        [[nodisard("ignoring commit status")]] bool commit();

        std::span<std::byte> data();
        bool write(std::span<const std::byte> buffer);
        void resize(size_t size);
        void produce(size_t byte_count);

    private:
        stream_producer* producer_;
        stream_sequence  producer_sequence_;
    }:

    class stream_consumer_transaction {
        friend class stream_consumer;

        stream_consumer_transaction(stream_consumer& consumer);

    public:
        stream_consumer_transaction(stream_consumer_transaction&& other);
        stream_consumer_transaction(const stream_consumer_transaction& other) = delete;
        stream_consumer_transaction& operator=(stream_consumer_transaction&& other);
        stream_consumer_transaction& operator=(const stream_consumer_transaction& other) = delete;
        ~stream_consumer_transaction();

        bool is_aborted() const;
        explicit operator bool() const;

        void rollback();
        [[nodisard("ignoring commit status")]] bool commit();

        std::span<const std::byte> data() const;
        std::optional<std::span<const std::byte>> read(size_t byte_count);
        void consume(size_t byte_count);

    private:
        stream_consumer*               consumer_;
        std::shared_ptr<stream_buffer> buffer_;
        stream_sequence                consumer_sequence_;
        stream_sequence                producer_sequence_;
    };

    class stream_producer {
    public:
        stream_producer(stream& stream);
        ~stream_producer();

        stream_sequence sequence() const;
        bool has_active_transaction() const;
        [[nodiscard]] stream_producer_transaction make_transaction();

    private:
        friend class stream;

        void abandon();

    private:
        stream*                      stream_;
        stream_producer_transaction* transaction_;
    };

    class stream_consumer {
    public:
        stream_consumer(stream& s);
        ~stream_consumer();

        stream_sequence sequence() const;
        bool has_active_transaction() const;
        [[nodiscard]] stream_consumer_transaction make_transaction();

    private:
        friend class stream;

        void abandon();

    private:
        stream*                      stream_;
        stream_sequence              sequence_;
        stream_consumer_transaction* transaction_;
    };

    class stream_observer {
        stream_observer(stream_observer&&) = delete;
        stream_observer(const stream_observer&) = delete;
        stream_observer& operator=(stream_observer&&) = delete;
        stream_observer& operator=(const stream_observer&) = delete;

    public:
        stream_observer(stream& s, stream_event event_mask);
        virtual ~stream_observer();

        stream_event event_mask() const;

        virtual void on_stream_event(stream_event event) = 0;

    private:
        friend class stream;

        void abandon();

    private:
        stream*      stream_;
        stream_event event_mask_;
    };
}
