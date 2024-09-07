#pragma once

#include "slag/core.h"
#include "slag/object.h"
#include "slag/resource.h"
#include "slag/topology.h"

#include <optional>
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

    using Message = Resource<ResourceType::MESSAGE>;

    struct Address {
        uint32_t valid         :  1                      = 0;
        uint32_t reserved      :  1                      = 0;
        uint32_t thread_index  : 10                      = 0;
        uint32_t channel_nonce : SLAG_CHANNEL_NONCE_BITS = 0;
        uint32_t channel_index                           = 0;
    };
    static_assert(sizeof(Address) == 8);

    template<>
    class Resource<ResourceType::MESSAGE> : public Object {
    public:
        Resource()
            : Object(static_cast<ObjectGroup>(ResourceType::MESSAGE))
        {
        }

        Address origin() const {
            return origin_;
        }

    private:
        friend class Router;

        bool bind(Address origin) {
            if (origin_.valid) {
                return false; // Already bound.
            }

            origin_ = origin;
            return true;
        }

    private:
        Address origin_;
    };

    struct alignas(32) Packet {
        Address      src_addr;
        Address      dst_addr;
        ThreadRoute  route;
        Ref<Message> msg;
    };
    static_assert(sizeof(Packet) == 32);

    class alignas(64) Link {
    public:
        void send(Packet packet);
        std::optional<Packet> receive();

    private:
        std::mutex         mutex_;
        std::deque<Packet> packets_;
    };

    class Fabric : public Managed {
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

    private:
        ThreadGraph                        thread_topology_;
        std::vector<std::unique_ptr<Link>> links_;
    };

    class Router {
    public:
        Router(Fabric& fabric, ThreadIndex thread_index);

        void attach(Channel& channel);
        void detach(Channel& channel);

        void send(Channel& channel, Address dst_addr, Ref<Message> message);
        Ptr<Message> receive(Channel& channel);

        void finalize(Message& message);

    private:
        void deliver(Packet packet);
        void forward(Packet packet);

    private:
        friend class Channel;

        struct PacketQueue {
            Event              event;
            std::deque<Packet> queue;
        };

        struct ChannelState {
            uint32_t    nonce = 0;

            Event       ready;
            uint32_t    outstanding_message_count = 0;
            uint32_t    outstanding_message_limit = 0;

            PacketQueue rx_queue;
            PacketQueue tx_queue;
        };

        ChannelState& get_state(const Channel& channel);
        ChannelState* get_state(Address address);

    private:
        Fabric&                   fabric_;
        ThreadIndex               thread_index_;
        ThreadRouteTable          thread_routes_;

        std::vector<Link*>        tx_links_;
        ThreadMask                tx_link_mask_;
        std::vector<Link*>        rx_links_;
        ThreadMask                rx_link_mask_;

        std::vector<ChannelState> channel_states_;
        std::vector<uint32_t>     unused_channel_states_;
    };

    class Channel final
        : public Pollable<PollableType::READABLE>
        , public Pollable<PollableType::WRITABLE>
    {
    public:
        Channel();
        ~Channel();

        Channel(Channel&&) = delete;
        Channel(const Channel&) = delete;
        Channel& operator=(Channel&&) = delete;
        Channel& operator=(const Channel&) = delete;

        Address address() const;

        Event& readable_event() override;
        Event& writable_event() override;

        void send(Address dst_addr, Ref<Message> msg);
        Ptr<Message> receive();

    private:
        friend class Router;

        void bind(Address address);

    private:
        Router& router_;
        Address address_;
    };

}
