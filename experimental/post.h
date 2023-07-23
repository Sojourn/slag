#pragma once

#include <cstdint>
#include <cstddef>

namespace slag {

    // Ring should be ordered based on sorted IP address
    //   should be rack/region oblivious
    //   primarily a broadcast/delivery mechanism
    //   also deliver subscriptions quickly, which are just messages that get broadcast
    //   should there be fanout (MPI diagram for broadcast)
    //     advantages of a single node orchestrating?
    //       sending into equal-cost points in the ring
    //       unified cost model of traversing the ring
    //       can better compensate for nodes with very little tx that are near the root
    //          aka don't make the leader propagate subscriptions
    //     max cost?
    //     simplicity?
    //   algorithm
    //     
    //   ally-oop (deputize node by transferring control for fanout)
    //     include cost information which 
    //     have this be static-ish (routes)
    //     have gossip create/destroy routes to self-heal/optimize
    //   gossip about cost and route information
    //     route information could be inferred on every node based on cost information from gossip
    //     assume everyone will start using the new nodes
    //     need to think about cost getting out of sink and edge cases around that

    // Maintains a list of subscriptions
    //   will attempt to send subscribe requests to the mailing list if a message hasn't been received in awhile

    // Membership token ring network
    //   random token spawn if one hasn't been received
    //   node with a token unicast to neighbor

    // A message can act like a token
    //   attempts to fire the token into multiple points in the ring based on link speeds
    //     optimal fanout latency based on scheduled resources
    //     feedback loop between multiple TCP connections
    //     
    //

    // turtle and hare for an individual token
    //    can be in multiple places in the ring due to retries/duplicates
    //    if a neighbor sees a stale token, it can detect problems on earlier links with some probability
    //    also can detect if an intermediate node drops out w/o waiting for all the retries to complete
    //      speed up the token's progress
    //        constant progress
    //    should include a report of all the actions nodes tried to take
    //    two-phase? probe,probe_response,

    // limited envelope payload means we might only be able to gossip about part of the ring at a time
    //   only communicate subscription deltas at a limited rate
    //   gossip can probably be a bit hierarical (node/thread/task)
    //

    // flood operation instead of a strict broadcast to reply from a more local
    //   copy of the message
    //

    // cycle detection and ttl to mitigate problems (destroy looping routes)

    // Uniquely describes a PostOffice in the system
    struct PostArea {
        uint16_t machine; // not an address
        uint16_t process; // not a pid/tid
    };

    // Uniquely describes a PostBox.
    struct PostCode : PostArea {
        uint32_t index; // TODO: workshop this name
    };

    struct Stamp {
        uint64_t sequence     : 56; // wraps; but should be unique while in flight
        uint64_t time_to_live : 5;
        uint64_t acknowlege   : 1;
        uint64_t delivered    : 1;
        uint64_t returning    : 1;
    };

    struct EnvelopeHeader {
        Stamp    stamp; // 8
        PostCode to;    // 8
        PostCode from;  // 8
    };
    static_assert(sizeof(EnvelopeHeader) == 24);

    // Does not own the underlying buffer.
    struct BufferDescriptor {
        uint32_t buffer_group : 8;
        uint32_t buffer_index : 24;
    };

    // Is the unique owner of a buffer descriptor.
    class BufferHandle {
    public:
        BufferHandle();
        BufferHandle(BufferDescriptor descriptor);
        BufferHandle(BufferHandle&& other);
        BufferHandle(const BufferHandle&) = delete;
        ~BufferHandle();

        BufferHandle& operator=(BufferHandle&& rhs);
        BufferHandle& operator=(const BufferHandle&) = delete;

        // Grant temporary ownership.
        [[nodiscard]] BufferDescriptor borrow();

        // Transition to manual ownership.
        [[nodiscard]] BufferDescriptor release();

        void reset();

    private:
        BufferDescriptor descriptor_;
    };

    // TODO: think about making a dynamic variant of this
    template<typename T, size_t capacity_>
    class CircularQueue {
    public:
        CircularQueue();
        CircularQueue(const CircularQueue& other);
        CircularQueue(CircularQueue&& other);
        ~CircularQueue();

        CircularQueue& operator=(const CircularQueue& other);
        CircularQueue& operator=(CircularQueue&& other);

        bool is_empty() const;
        bool is_full() const;
        size_t size() const;
        size_t capacity() const;

        template<typename... Args>
        bool push(Args&&... args);
        std::optional<T> pop();

    private:
        class Slot {
        public:
            template<typename... Args>
            void create(Args&&... args);
            void destroy();

            [[nodiscard]] T& get();

        private:
            alignas(64) std::array<std::byte, sizeof(T)> storage_;
        };

    private:
        uint32_t                    head_;
        uint32_t                    tail_;
        std::array<Slot, capacity_> data_;
    };

    class Envelope {
    public:
        Envelope(const EnvelopeHeader& header, BufferHandle contents);

    private:
        EnvelopeHeader header_;
        BufferHandle   contents_;
    };

    class PostBox {
    public:
        explicit PostBox(PostOffice& post_office);
        PostBox(PostBox&& other);
        PostBox(const PostBox& other) = delete;
        ~PostBox();

        PostBox& operator=(PostBox&& rhs);
        PostBox& operator=(const PostBox&) = delete;

        const PostCode& address() const;

        // Used by the owner of a PostBox to tell when an Envelope can be received.
        Pollable& incoming();

        // Used by the owner of a PostBox to tell when an Envelope can be sent.
        Pollable& outgoing();

        void send(PostCode to, BufferHandle content);
        void reply(const Envelope& envelope, BufferHandle content);   // for rpc
        void forward(PostCode to, const Envelope& envelope); // for brokering
        const Envelope* receive();
    };

    class Pollable {
    public:
        Pollable(PollableObserver* observer);

        [[nodiscard]] bool is_ready() const;
        void set_ready();
        void reset_ready();

        [[nodiscard]] bool is_muted() const;
        void set_muted();
        void reset_muted();

        void* user_data();
        const void* user_data() const;
        void set_user_data(void* user_data);
    };

    class PollableObserver {
    public:
        virtual ~PollableObserver() = default;

        virtual void handle_pollable_ready(PollableObserver* observer) = 0;
        virtual void handle_pollable_destroyed(PollableObserver* observer) = 0;
    };

    class Selector : public Pollable, private PollableObserver {
    public:
        void insert(T& pollable);
        void remove(T& pollable);

        Pollable* poll();

    private:
        void handle_pollable_ready(PollableObserver* observer) override final;
        void handle_pollable_destroyed(PollableObserver* observer) override final;
    };

    struct PostalTransport {
        // IPv4/IPv6 address, spsc queue
    };

    class PostalGraph {
    public:
        void add_node(const PostArea& area) {
        }

        void add_edge(const PostArea& to, const PostArea& from, const PostalTransport& transport) {
        }

    private:
        using NodeIndex = uint32_t;
        using EdgeIndex = uint32_t;

        struct Edge {
            PostArea        to;
            PostArea        from;
            PostalTransport transport;
        };

        struct Node {
            PostArea               area;
            std::vector<EdgeIndex> edges;
        };

        std::vector<Node>                       nodes_;
        std::vector<Edge>                       edges_;
        std::unordered_map<PostArea, NodeIndex> node_index_;
    };

    // Maintains a set of PostBoxes, and facilitates pickup and delivery of envelopes.
    class PostOffice {
    public:
        explicit PostOffice(PostArea area);

    private:
        PostArea area_;
    };

}
