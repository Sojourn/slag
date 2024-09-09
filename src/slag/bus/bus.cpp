#include "slag/bus.h"
#include "slag/context.h"

namespace slag {

    void Link::send(Packet packet) {
        std::scoped_lock lock(mutex_);
        packets_.push_back(packet);
    }

    std::optional<Packet> Link::receive() {
        std::scoped_lock lock(mutex_);
        if (packets_.empty()) {
            return std::nullopt;
        }            

        Packet packet = packets_.front();
        packets_.pop_front();
        return packet;
    }

    Router::Router(std::shared_ptr<Fabric> fabric, ThreadIndex thread_index)
        : fabric_(fabric)
        , thread_index_(thread_index)
        , thread_routes_(fabric->routes(thread_index))
        , tx_links_(MAX_THREAD_COUNT)
        , tx_link_mask_(0)
        , rx_links_(MAX_THREAD_COUNT)
        , rx_link_mask_(0)
    {
        for (ThreadIndex thread_index = 0; thread_index < MAX_THREAD_COUNT; ++thread_index) {
            if (Link* link = fabric_->link(thread_index_, thread_index)) {
                tx_links_[thread_index] = link;
                tx_link_mask_ |= (1ull << thread_index);
            }
            else {
                tx_links_[thread_index] = nullptr;
            }

            if (Link* link = fabric_->link(thread_index, thread_index_)) {
                rx_links_[thread_index] = link;
                rx_link_mask_ |= (1ull << thread_index);
            }
            else {
                rx_links_[thread_index] = nullptr;
            }
        }
    }

    void Router::attach(Channel& channel) {
        ChannelState* state = nullptr;
        size_t channel_index = 0;

        if (unused_channel_states_.empty()) {
            channel_index = channel_states_.size();
            state = &channel_states_.emplace_back();
        }
        else {
            channel_index = unused_channel_states_.back();
            unused_channel_states_.pop_back();
            state = &channel_states_[channel_index];
        }

        channel.bind(ChannelId {
            .valid         = 1,
            .reserved      = 0,
            .thread_index  = thread_index_,
            .channel_nonce = state->nonce,
            .channel_index = static_cast<uint32_t>(channel_index),
        });
    }

    void Router::detach(Channel& channel) {
        ChannelState& state = get_state(channel);

        for (PacketQueue* packet_queue : {&state.rx_queue, &state.tx_queue}) {
            packet_queue->event.reset();
            packet_queue->queue.clear();
        }

        state.ready.set();
        state.outstanding_message_count = 0;
        state.outstanding_message_limit = 1024; // TODO: Make this configurable.

        state.nonce += 1;
        if (state.nonce < (1ull << SLAG_CHANNEL_NONCE_BITS)) {
            unused_channel_states_.push_back(channel.id().channel_index);
        }
        else {
            // Retire this state instead of repeating a nonce.
        }
    }

    void Router::send(Channel& channel, ChannelId dst_chid, Ref<Message> message) {
        if (message->bind(channel.id())) {
            ChannelState& src_state = get_state(channel);
            src_state.outstanding_message_count += 1;
            if (src_state.outstanding_message_count >= src_state.outstanding_message_limit) {
                src_state.ready.reset();
            }
        }

        Packet packet = {
            .src_chid = channel.id(),
            .dst_chid = dst_chid,
            .route    = thread_routes_[dst_chid.thread_index],
            .msg      = message,
        };

        if (thread_index_ == dst_chid.thread_index) {
            deliver(packet);
        }
        else {
            forward(packet);
        }
    }

    Ptr<Message> Router::receive(Channel& channel) {
        ChannelState& state = get_state(channel);
        if (state.rx_queue.queue.empty()) {
            return {};
        }

        Ptr<Message> msg = state.rx_queue.queue.front().msg;
        state.rx_queue.queue.pop_front();
        if (state.rx_queue.queue.empty()) {
            state.rx_queue.event.reset();
        }

        return msg;
    }

    void Router::finalize(Message& message) {
        // Not sent or
        assert(!message.origin().valid || (message.origin().thread_index == thread_index_));

        if (ChannelState* state = get_state(message.origin())) {
            assert(state->outstanding_message_count > 0);

            state->outstanding_message_count -= 1;
            if (state->outstanding_message_count < state->outstanding_message_limit) {
                state->ready.set();
            }
        }

        delete &message;
    }

    void Router::forward(Packet packet) {
        assert(thread_index_ != packet.dst_chid.thread_index);

        if (std::optional<ThreadIndex> next_thread_index = packet.route.next_hop(thread_index_)) {
            if (Link* link = tx_links_[*next_thread_index]) {
                link->send(packet);
            }
            else {
                abort(); // Invalid route.
            }
        }
        else {
            // No route.
        }
    }

    void Router::deliver(Packet packet) {
        assert(thread_index_ == packet.dst_chid.thread_index);

        if (ChannelState* state = get_state(packet.dst_chid); LIKELY(state)) {
            state->rx_queue.event.set();
            state->rx_queue.queue.push_back(packet);
        }
        else {
            // The channel was destroyed.
        }
    }

    auto Router::get_state(const Channel& channel) -> ChannelState& {
        const ChannelId chid = channel.id();
        assert(chid.valid);
        assert(chid.reserved == 0);
        assert(chid.thread_index == thread_index_);
        assert(chid.channel_index < channel_states_.size());

        ChannelState& state = channel_states_[chid.channel_index];
        assert(chid.channel_nonce == state.nonce);

        return state;
    }

    auto Router::get_state(ChannelId chid) -> ChannelState* {
        bool is_safe = true;
        is_safe &= chid.valid;
        is_safe &= chid.reserved == 0;
        is_safe &= chid.thread_index == thread_index_;
        is_safe &= chid.channel_index < channel_states_.size();
        if (!is_safe) {
            return nullptr;
        }

        ChannelState& state = channel_states_[chid.channel_index];
        if (chid.channel_nonce != state.nonce) {
            return nullptr;
        }

        return &state;
    }

    Channel::Channel()
        : router_(get_router())
    {
        router_.attach(*this);
    }

    Channel::~Channel() {
        router_.detach(*this);
    }

    ChannelId Channel::id() const {
        return id_;
    }

    Event& Channel::readable_event() {
        Router::ChannelState& state = router_.get_state(*this);

        return state.rx_queue.event;
    }

    Event& Channel::writable_event() {
        Router::ChannelState& state = router_.get_state(*this);

        return state.ready;
    }

    void Channel::send(ChannelId dst_chid, Ref<Message> msg) {
        router_.send(*this, dst_chid, msg);
    }

    Ptr<Message> Channel::receive() {
        return router_.receive(*this);
    }

    void Channel::bind(ChannelId chid) {
        id_ = chid;
    }

}
