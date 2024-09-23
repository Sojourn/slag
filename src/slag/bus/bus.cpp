#include "slag/bus.h"
#include "slag/context.h"
#include "slag/runtime.h"

namespace slag {

    Router::Router(std::shared_ptr<Fabric> fabric, ThreadIndex thread_index)
        : fabric_(fabric)
        , thread_index_(thread_index)
        , thread_routes_(fabric->routes(thread_index))
        , rx_links_(MAX_THREAD_COUNT)
        , rx_link_mask_(0)
        , tx_links_(MAX_THREAD_COUNT)
        , tx_link_mask_(0)
        , tx_send_mask_(0)
        , temp_packet_array_(512)
    {
        for (ThreadIndex remote_thread_index = 0; remote_thread_index < MAX_THREAD_COUNT; ++remote_thread_index) {
            if (Link* link = fabric_->link(remote_thread_index, thread_index_)) {
                rx_links_[remote_thread_index] = SpscQueueConsumer<Packet>(link->queue());
                rx_link_mask_ |= (1ull << remote_thread_index);
            }

            if (Link* link = fabric_->link(thread_index_, remote_thread_index)) {
                tx_links_[remote_thread_index] = SpscQueueProducer<Packet>(link->queue());
                tx_link_mask_ |= (1ull << remote_thread_index);
            }
        }
    }

    Event& Router::writable_event() {
        return tx_backlog_event_;
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

        const ChannelId chid = {
            .valid         = 1,
            .reserved      = 0,
            .thread_index  = thread_index_,
            .channel_nonce = state->nonce,
            .channel_index = static_cast<uint32_t>(channel_index),
        };

        try {
            if (auto&& name = channel.name()) {
                fabric_->bind_channel(*name, chid);
            }
        }
        catch (const std::exception&) {
            unused_channel_states_.push_back(channel_index);
            throw;
        }

        channel.bind(chid);
    }

    void Router::detach(Channel& channel) {
        ChannelState& state = get_state(channel);

        state.writable_event.set();
        state.outstanding_message_count = 0;
        state.outstanding_message_limit = ChannelState::DEFAULT_OUTSTANDING_MESSAGE_LIMIT;

        state.rx_queue.event.reset();
        state.rx_queue.queue.clear();

        state.nonce += 1;
        if (state.nonce < (1ull << SLAG_CHANNEL_NONCE_BITS)) {
            unused_channel_states_.push_back(channel.id().channel_index);
        }
        else {
            // Retire this state instead of repeating a nonce.
        }

        if (auto&& name = channel.name()) {
            fabric_->unbind_channel(*name);
        }
    }

    std::optional<ChannelId> Router::query(const std::string& name) const {
        return fabric_->find_channel(name);
    }

    void Router::send(Channel& channel, ChannelId dst_chid, Ref<Message> message) {
        if (message->bind(channel.id())) {
            ChannelState& src_state = get_state(channel);
            src_state.outstanding_message_count += 1;
            if (src_state.outstanding_message_count >= src_state.outstanding_message_limit) {
                src_state.writable_event.reset();
            }
        }

        route(Packet {
            .src_chid  = channel.id(),
            .dst_chid  = dst_chid,
            .route     = thread_routes_[dst_chid.thread_index],
            .hop_index = 0,
            .reserved  = {0},
            .msg       = message,
        });
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

    bool Router::poll(const ThreadMask sources) {
        size_t total_packet_count = 0;

        for_each_thread(sources & rx_link_mask_, [&](const ThreadIndex thread_index) {
            std::span<Packet*> packets = {
                temp_packet_array_.data(),
                temp_packet_array_.size(),
            };

            if (size_t packet_count = rx_links_[thread_index].poll(packets)) {
                for (size_t packet_index = 0; packet_index < packet_count; ++packet_index) {
                    route(*packets[packet_index]);
                }

                total_packet_count += packet_count;
            }
        });

        return total_packet_count > 0;
    }

    void Router::flush() {
        // Flush packets from the backlog.
        {
            while (!tx_backlog_.empty()) {
                Packet& packet = tx_backlog_.front();
                if (forward(packet)) {
                    tx_backlog_.pop_front();
                }
                else {
                    break; // The link is at capacity.
                }
            }

            tx_backlog_event_.set(!tx_backlog_.empty());
        }

        // Publish and notify threads that their end of the links can be read.
        {
            // Wake threads we've sent to using `LINK` interrupts.
            for_each_thread(tx_send_mask_, [&](const ThreadIndex tx_tidx) {
                tx_links_[tx_tidx].flush();

                std::cout << thread_index_ << " interrupting " << tx_tidx << std::endl;
                start_interrupt_operation(
                    get_runtime().reactor(tx_tidx),
                    Interrupt {
                        .source = thread_index_,
                        .reason = InterruptReason::LINK,
                    }
                )->daemonize();
            });

            tx_send_mask_ = 0;
        }
    }

    void Router::finalize(Message& message) {
        assert(!message.origin().valid || (message.origin().thread_index == thread_index_));

        if (ChannelState* state = get_state(message.origin())) {
            assert(state->outstanding_message_count > 0);

            state->outstanding_message_count -= 1;
            if (state->outstanding_message_count < state->outstanding_message_limit) {
                state->writable_event.set();
            }
        }

        delete &message;
    }

    void Router::route(Packet packet) {
        if (thread_index_ == packet.dst_chid.thread_index) {
            deliver(packet);
        }
        else if (tx_backlog_.empty() && forward(packet)) {
            // The packet was sent.
        }
        else {
            tx_backlog_.push_back(packet);
            tx_backlog_event_.set();
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

    bool Router::forward(Packet packet) {
        assert(thread_index_ != packet.dst_chid.thread_index);

        if (const ThreadIndex next_tidx = packet.route.hop(packet.hop_index++); next_tidx != INVALID_THREAD_INDEX) {
            assert(thread_index_ != next_tidx);
            assert(next_tidx < MAX_THREAD_COUNT);

            SpscQueueProducer<Packet>& producer = tx_links_[next_tidx];
            if (producer.insert(packet)) {
                tx_send_mask_ |= (1ull << next_tidx);
                return true;
            }
            else {
                // Link is at capacity.
            }
        }
        else {
            // No route.
        }

        return false;
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

    Channel::Channel(const std::string& name)
        : router_(get_router())
        , name_(name)
    {
        router_.attach(*this);
    }

    Channel::~Channel() {
        router_.detach(*this);
    }

    ChannelId Channel::id() const {
        return id_;
    }

    const std::optional<std::string>& Channel::name() const {
        return name_;
    }

    Event& Channel::readable_event() {
        Router::ChannelState& state = router_.get_state(*this);

        return state.rx_queue.event;
    }

    Event& Channel::writable_event() {
        Router::ChannelState& state = router_.get_state(*this);

        return state.writable_event;
    }

    std::optional<ChannelId> Channel::query(const std::string& name) const {
        return router_.query(name);
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
