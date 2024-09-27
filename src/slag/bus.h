#pragma once

#include "slag/object.h"
#include "slag/resource.h"
#include "slag/core.h"
#include "slag/system.h"
#include "slag/topology.h"
#include "slag/collections/spsc_queue.h"

#include <optional>
#include <string>
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

    struct alignas(32) Packet {
        ChannelId    src_chid;
        ChannelId    dst_chid;
        ThreadRoute  route;
        uint8_t      hop_index;
        uint8_t      reserved[3];
        Ref<Message> msg;
    };
    static_assert(sizeof(Packet) == 32);

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
        explicit Fabric(const ThreadGraph& thread_topology)
            : thread_topology_(thread_topology)
            , links_(thread_topology_.edge_count())
        {
            for (auto& link :  links_) {
                link = std::make_unique<Link>();
            }
        }

        ThreadRouteTable routes(ThreadIndex thread_index) const {
            return build_thread_route_table(thread_topology_, thread_index);
        }

        Link* link(ThreadIndex src_thread, ThreadIndex dst_thread) {
            const ThreadGraph::Edge edge = {
                .source = src_thread,
                .target = dst_thread,
            };

            if (std::optional<size_t> edge_index = thread_topology_.edge_index(edge)) {
                return links_.at(*edge_index).get();
            }
            else {
                return nullptr;
            }
        }

        std::optional<ChannelId> find_channel(const std::string& name) const {
            std::scoped_lock lock(channel_mutex_);

            if (auto it = channel_table_.find(name); it != channel_table_.end()) {
                return it->second;
            }

            return std::nullopt;
        }

        void bind_channel(const std::string& name, ChannelId chid) {
            std::scoped_lock lock(channel_mutex_);

            ChannelId& row = channel_table_[name];
            if (UNLIKELY(row.valid)) {
                throw std::runtime_error("Channel already bound to name");
            }

            row = chid;
        }

        void unbind_channel(const std::string& name) {
            std::scoped_lock lock(channel_mutex_);

            auto it = channel_table_.find(name);
            assert(it != channel_table_.end());
            channel_table_.erase(it);
        }

    private:
        const ThreadGraph                          thread_topology_;
        std::vector<std::unique_ptr<Link>>         links_;

        mutable std::mutex                         channel_mutex_;
        std::unordered_map<std::string, ChannelId> channel_table_;
    };

    class Router final
        : public Pollable<PollableType::WRITABLE>
    {
    public:
        Router(std::shared_ptr<Fabric> fabric, ThreadIndex thread_index);

        virtual ~Router() = default;

        // This event is set when the router is ready to be flushed.
        Event& writable_event() override;

        void attach(Channel& channel);
        void detach(Channel& channel);

        std::optional<ChannelId> query(const std::string& name) const;
        void send(Channel& channel, ChannelId dst_chid, Ref<Message> message);
        Ptr<Message> receive(Channel& channel);

        // Returns true if any packets were processed.
        bool poll(ThreadMask sources = (MAX_THREAD_COUNT - 1));

        void flush();

        // TODO: create_message/destroy_message API to match the reactor.
        void finalize(Message& message);

    private:
        void route(Packet packet);
        void deliver(Packet packet);
        bool forward(Packet packet);

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
        ThreadRouteTable                       thread_routes_;

        std::vector<SpscQueueConsumer<Packet>> rx_links_;
        ThreadMask                             rx_link_mask_;

        std::vector<SpscQueueProducer<Packet>> tx_links_;
        ThreadMask                             tx_link_mask_;
        ThreadMask                             tx_send_mask_;

        Event                                  tx_pending_event_;
        std::deque<Packet>                     tx_backlog_;

        std::vector<ChannelState>              channel_states_;
        std::vector<uint32_t>                  unused_channel_states_;
        std::vector<Packet*>                   temp_packet_array_;

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
