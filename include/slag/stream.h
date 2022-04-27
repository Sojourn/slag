#pragma once

#include <span>
#include <optional>
#include <vector>
#include <unordered_map>
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

    enum class stream_transaction_state {
        ACTIVE,
        ABORTED,
        ROLLED_BACK,
        COMMITTED,
    };

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

        void add_observer(stream_observer& observer, stream_event event_mask);
        void remove_observer(stream_observer& observer);

    private:
        friend class stream_producer;

        stream_buffer& buffer();
        size_t producer_segment_size() const;
        std::span<std::byte> producer_segment();
        void resize_producer_segment(size_t minimum_size);
        void advance_producer_sequence(size_t byte_count);

        void add_producer(stream_producer& producer);
        void remove_producer(stream_producer& producer);
        size_t active_producer_transaction_count() const;

    private:
        friend class stream_consumer;

        const stream_buffer& buffer() const;
        size_t consumer_segment_size() const;
        std::span<const std::byte> consumer_segment() const;
        void update_consumer_sequence();

        void add_consumer(stream_consumer& consumer);
        void remove_consumer(stream_consumer& consumer);
        size_t active_consumer_transaction_count() const;
    
    private:
        friend class stream_observer;

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

        stream_transaction_state state() const;
        bool is_aborted() const;
        explicit operator bool() const;

        void rollback();
        [[nodisard("ignoring commit status")]] bool commit();

        std::span<std::byte> data();
        bool write(std::span<const std::byte> buffer);
        void resize(size_t size);
        void produce(size_t byte_count);

    private:
        friend class stream;
        friend class stream_producer;

        void set_state(stream_transaction_state state);
        void set_buffer(std::shared_ptr<stream_buffer> buffer);

    private:
        stream_buffer& buffer();

    private:
        stream_producer*               producer_;
        stream_transaction_state       state_;
        stream_sequence                producer_sequence_;
        std::shared_ptr<stream_buffer> buffer_;
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

        stream_transaction_state state() const;
        bool is_aborted() const;
        explicit operator bool() const;

        void rollback();
        [[nodisard("ignoring commit status")]] bool commit();

        std::span<const std::byte> data() const;
        std::optional<std::span<const std::byte>> read(size_t byte_count);
        void consume(size_t byte_count);

    private:
        friend class stream;
        friend class stream_producer;

        void set_state(stream_transaction_state state);
        void set_buffer(std::shared_ptr<stream_buffer> buffer);
    
    private:
        const stream_buffer& buffer() const;

    private:
        stream_consumer*               consumer_;
        stream_transaction_state       state_;
        stream_sequence                consumer_sequence_;
        stream_sequence                producer_sequence_;
        std::shared_ptr<stream_buffer> buffer_;
    };

    class stream_producer {
    public:
        stream_producer(stream& stream);
        ~stream_producer();

        [[nodiscard]] stream_producer_transaction make_transaction();

    private:
        friend class stream;

        stream_producer_transaction* transaction();
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
        friend class stream_consumer_transaction;

        void transaction_complete();

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
        virtual ~stream_observer();

        virtual void on_stream_event(stream& subject, stream_event event) = 0;

    private:
        friend class stream;

        std::optional<stream_event> find_hook(const stream& subject) const;
        void add_hook(stream& subject, stream_event event_mask);
        void remove_hook(stream& subject);

    private:
        std::unordered_map<stream*, stream_event> hooks_;
    };
}
