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

    enum class stream_event {
        NONE,
        DATA_PRODUCED,
        DATA_CONSUMED,
    };

    std::string to_string(stream_event event_mask);
    bool operator&(stream_event l, stream_event r);
    void operator&=(stream_event& l, stream_event r);
    bool operator|(stream_event l, stream_event r);
    void operator|=(stream_event& l, stream_event r);

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

        void add_producer(stream_producer& producer);
        void remove_producer(stream_producer& producer);
        void advance_producer_sequence(size_t byte_count);
        size_t active_producer_transaction_count() const;

    private:
        friend class stream_consumer;

        void add_consumer(stream_consumer& consumer);
        void remove_consumer(stream_consumer& consumer);
        void update_consumer_sequence();
        size_t active_consumer_transaction_count() const;
    
    private:
        friend class stream_observer;

        void add_observer(stream_observer& observer);
        void update_observer_event_mask(stream_observer& observer, stream_event new_event_mask);
        void remove_observer(stream_observer& observer);
        void notify_observers(stream_event event);
        std::vector<stream_observer*>& event_observers(stream_event event);

    private:
        std::shared_ptr<stream_buffer> buffer_;
        uint64_t                       producer_sequence_;
        uint64_t                       consumer_sequence_;
        std::vector<stream_producer*>  producers_;
        std::vector<stream_consumer*>  consumers_;
        std::vector<stream_observer*>  producer_observers_;
        std::vector<stream_observer*>  consumer_observers_;
    };

    class stream_transaction_aborted : public std::exception {
    public:
        using std::exception::exception;

        const char* what() const noexcept override {
            return "stream transaction aborted";
        }
    };

    class stream_producer_transaction {
    public:
        stream_producer_transaction(stream_producer& producer);
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
        uint64_t         producer_sequence_;
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
        uint64_t                       consumer_sequence_;
        uint64_t                       producer_sequence_;
    };

    class stream_producer {
    public:
        stream_producer(stream& stream);
        ~stream_producer();

        uint64_t sequence() const;
        bool has_active_transaction() const;
        [[nodiscard]] stream_producer_transaction make_transaction();

    private:
        stream&                      stream_;
        stream_producer_transaction* transaction_;
    };

    class stream_consumer {
    public:
        stream_consumer(stream& s);
        ~stream_consumer();

        uint64_t sequence() const;
        bool has_active_transaction() const;
        [[nodiscard]] stream_consumer_transaction make_transaction();

    private:
        stream&                      stream_;
        uint64_t                     sequence_;
        stream_consumer_transaction* transaction_;
    };

    class stream_observer {
        stream_observer(stream_observer&&) = delete;
        stream_observer(const stream_observer&) = delete;
        stream_observer& operator=(stream_observer&&) = delete;
        stream_observer& operator=(const stream_observer&) = delete;

    public:
        stream_observer(stream& s, stream_event event_mask = stream_event::NONE);
        virtual ~stream_observer();

        stream_event event_mask() const;
        void update_event_mask(stream_event event_mask);

        virtual void on_stream_data_produced() {}
        virtual void on_stream_data_consumed() {}

    private:
        stream*      stream_;
        stream_event event_mask_;
    };
}
