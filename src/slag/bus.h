#pragma once

#include "slag/object.h"
#include "slag/resource.h"
#include "slag/core.h"
#include "slag/system.h"
#include "slag/topology.h"
#include "slag/collections/spsc_queue.h"

#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <array>
#include <deque>
#include <vector>
#include <cstdint>
#include <cstddef>

#define SLAG_CHANNEL_NONCE_BITS 20

namespace slag {

    class Fabric;
    class Router;
    class Channel;

    struct ChannelId {
        uint32_t valid         :  1                      = 0;
        uint32_t reserved      :  1                      = 0;
        uint32_t thread_index  : 10                      = 0;
        uint32_t channel_nonce : SLAG_CHANNEL_NONCE_BITS = 0;
        uint32_t channel_index                           = 0;
    };
    static_assert(sizeof(ChannelId) == 8);

    template<>
    class Resource<ResourceType::MESSAGE> : public Object {
    public:
        Resource()
            : Object(static_cast<ObjectGroup>(ResourceType::MESSAGE))
        {
        }

        ChannelId origin() const {
            return origin_;
        }

    private:
        friend class Router;

        bool bind(ChannelId origin) {
            if (origin_.valid) {
                return false; // Already bound.
            }

            origin_ = origin;
            return true;
        }

    private:
        ChannelId origin_;
    };

    struct Packet {
        ChannelId    dst_chid;
        Ref<Message> msg;
    };
    static_assert(sizeof(Packet) == 16);

    class alignas(64) Link {
    public:
        Link()
            : queue_(16 * 1024) // TODO: Make this configurable.
        {
        }

        SpscQueue<Packet>& queue() {
            return queue_;
        }

    private:
        SpscQueue<Packet> queue_;
    };

    class Fabric {
    public:
        // Links between threads are created dynamically on first-use.
        Link& link(ThreadIndex src_thread, ThreadIndex dst_thread) {
            std::scoped_lock lock(mutex_);

            return link_table_[std::make_pair(src_thread, dst_thread)];
        }

        std::optional<ChannelId> find_channel(const std::string& name) const {
            std::scoped_lock lock(mutex_);

            if (auto it = channel_table_.find(name); it != channel_table_.end()) {
                return it->second;
            }

            return std::nullopt;
        }

        void bind_channel(const std::string& name, ChannelId chid) {
            std::scoped_lock lock(mutex_);

            ChannelId& row = channel_table_[name];
            if (UNLIKELY(row.valid)) {
                throw std::runtime_error("Channel already bound to name");
            }

            row = chid;
        }

        void unbind_channel(const std::string& name) {
            std::scoped_lock lock(mutex_);

            auto it = channel_table_.find(name);
            assert(it != channel_table_.end());
            channel_table_.erase(it);
        }

    private:
        using LinkTable = std::map<std::pair<ThreadIndex, ThreadIndex>, Link>;
        using ChannelTable = std::unordered_map<std::string, ChannelId>;

        mutable std::mutex mutex_;
        LinkTable          link_table_;
        ChannelTable       channel_table_;
    };

    class Router final
        : public Pollable<PollableType::READABLE> // Poll while set.
        , public Pollable<PollableType::WRITABLE> // Flush while set.
    {
    public:
        Router(std::shared_ptr<Fabric> fabric, ThreadIndex thread_index);

        virtual ~Router() = default;

        Event& readable_event() override;
        Event& writable_event() override;

        void attach(Channel& channel);
        void detach(Channel& channel);

        std::optional<ChannelId> query(const std::string& name) const;
        void send(Channel& channel, ChannelId dst_chid, Ref<Message> message);
        Ptr<Message> receive(Channel& channel);

        // Returns true if any packets were processed.
        bool poll();

        void flush();

        // TODO: create_message/destroy_message API to match the reactor.
        void finalize(Message& message);

    private:
        void route(const Packet& packet);
        void deliver(const Packet& packet);
        bool forward(const Packet& packet);

        void update_readiness();

    private:
        friend class Channel;
        friend class ChannelDriver;

        struct PacketQueue {
            Event              event; // Set when the queue is non-empty.
            std::deque<Packet> queue;
        };

        struct ChannelState {
            static constexpr uint32_t DEFAULT_OUTSTANDING_MESSAGE_LIMIT = 1024;

            uint32_t    nonce = 0;

            Event       writable_event;
            uint32_t    outstanding_message_count = 0;
            uint32_t    outstanding_message_limit = DEFAULT_OUTSTANDING_MESSAGE_LIMIT;

            PacketQueue rx_queue;
        };

        ChannelState& get_state(const Channel& channel);
        ChannelState* get_state(ChannelId chid);

        // Get or create.    
        SpscQueueConsumer<Packet>& get_rx_link(ThreadIndex tidx);
        SpscQueueProducer<Packet>& get_tx_link(ThreadIndex tidx);
        PacketQueue& get_tx_backlog(ThreadIndex tidx);

    private:
        struct Metrics {
            size_t deliver_count = 0;
            size_t route_count = 0;
            size_t forward_count = 0;
            size_t forward_success_count = 0;
            size_t forward_failure_count = 0;
            size_t send_count = 0;
            size_t receive_count = 0;
            size_t originate_count = 0;
            size_t interrupt_count = 0;
            size_t finalize_count = 0;
        };

        std::shared_ptr<Fabric>                fabric_;
        ThreadIndex                            thread_index_;

        std::vector<ChannelState>              channel_states_;
        std::vector<uint32_t>                  unused_channel_states_;

        Event&                                 rx_event_;
        ThreadMask&                            rx_event_mask_;
        std::vector<SpscQueueConsumer<Packet>> rx_links_;

        Event                                  tx_event_;
        ThreadMask                             tx_event_mask_;
        std::vector<SpscQueueProducer<Packet>> tx_links_;
        std::vector<PacketQueue>               tx_backlogs_;
        ThreadMask                             tx_backlog_mask_;

        Metrics                                metrics_;
    };

    class Channel final
        : public Pollable<PollableType::READABLE> // Pending data.
        , public Pollable<PollableType::WRITABLE> // Able to send (not back-pressured).
    {
    public:
        Channel();
        explicit Channel(const std::string& name);
        ~Channel();

        Channel(Channel&&) = delete;
        Channel(const Channel&) = delete;
        Channel& operator=(Channel&&) = delete;
        Channel& operator=(const Channel&) = delete;

        ChannelId id() const;
        const std::optional<std::string>& name() const;

        Event& readable_event() override;
        Event& writable_event() override;

        std::optional<ChannelId> query(const std::string& name) const;
        void send(ChannelId dst_chid, Ref<Message> msg);
        Ptr<Message> receive();

    private:
        friend class Router;

        void bind(ChannelId chid);

    private:
        Router& router_;
        ChannelId id_;
        std::optional<std::string> name_;
    };

}
