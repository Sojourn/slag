#include "slag/bus.h"
#include "slag/context.h"
#include "slag/runtime.h"

namespace slag {

    Router::Router(std::shared_ptr<Fabric> fabric, ThreadIndex thread_index)
        : fabric_(fabric)
        , thread_index_(thread_index)
        , rx_event_(get_reactor().interrupt_state(InterruptReason::LINK).event)
        , rx_event_mask_(get_reactor().interrupt_state(InterruptReason::LINK).sources)
        , tx_event_mask_(0)
    {
        assert(fabric_);

        rx_links_.reserve(MAX_THREAD_COUNT);
        tx_links_.reserve(MAX_THREAD_COUNT);
    }

    Event& Router::writable_event() {
        return tx_event_;
    }

    Event& Router::readable_event() {
        return rx_event_;
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

    void Router::send(Channel& channel, ChannelId dst_chid, Ref<Message> msg) {
        if (msg->bind(channel.id())) {
            ChannelState& src_state = get_state(channel);
            src_state.outstanding_message_count += 1;
            if (src_state.outstanding_message_count >= src_state.outstanding_message_limit) {
                src_state.writable_event.reset();
            }

            metrics_.originate_count += 1;
        }

        route(Packet {
            .dst_chid  = dst_chid,
            .msg       = msg,
        });

        metrics_.send_count += 1;
    }

    Ptr<Message> Router::receive(Channel& channel) {
        ChannelState& state = get_state(channel);
        if (state.rx_queue.queue.empty()) {
            return {};
        }

        Packet& packet = state.rx_queue.queue.front();
        Ptr<Message> msg = packet.msg;

        state.rx_queue.queue.pop_front();
        if (state.rx_queue.queue.empty()) {
            state.rx_queue.event.reset();
        }

        metrics_.receive_count += 1;

        return msg;
    }

    bool Router::poll() {
        size_t total_packet_count = 0;

        for_each_thread(rx_event_mask_, [&](const ThreadIndex rx_tidx) {
            Packet* packets[64];

            if (const size_t packet_count = get_rx_link(rx_tidx).poll(packets)) {
                for (size_t packet_index = 0; packet_index < packet_count; ++packet_index) {
                    const Packet* packet = packets[packet_index];
                    assert(packet);
                    deliver(*packet);
                }

                total_packet_count += packet_count;
            }
            else {
                rx_event_mask_ &= ~(1ull << rx_tidx);
            }
        });

        rx_event_.set(rx_event_mask_ != 0);

        return total_packet_count > 0;
    }

    void Router::flush() {
        // Flush packets from TX backlogs.
        for_each_thread(tx_backlog_mask_, [&](const ThreadIndex tx_tidx) {
            PacketQueue& tx_backlog = get_tx_backlog(tx_tidx);
            while (!tx_backlog.queue.empty()) {
                if (forward(tx_backlog.queue.front())) {
                    tx_backlog.queue.pop_front();
                }
                else {
                    break; // The link is at capacity.
                }
            }

            // Update the backlog mask if it was cleared.
            if (tx_backlog.queue.empty()) {
                tx_backlog_mask_ &= ~(1ull << tx_tidx);
            }
        });

        for_each_thread(tx_event_mask_, [&](const ThreadIndex tx_tidx) {
            // Ensure all writes are visible.
            get_tx_link(tx_tidx).flush();

            // FIXME: We should probably check the result of this and retry to avoid deadlocking.
            start_interrupt_operation(
                get_runtime().reactor(tx_tidx),
                Interrupt {
                    .source = thread_index_,
                    .reason = InterruptReason::LINK,
                }
            )->daemonize();

            metrics_.interrupt_count += 1;
        });

        tx_event_mask_ = 0;

        update_readiness();
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

        metrics_.finalize_count += 1;
    }

    void Router::route(const Packet& packet) {
        if (thread_index_ == packet.dst_chid.thread_index) {
            deliver(packet);
            return;
        }

        PacketQueue& tx_backlog = get_tx_backlog(packet.dst_chid.thread_index);
        if (tx_backlog.queue.empty() && forward(packet)) {
            assert(tx_event_mask_ & (1ull << packet.dst_chid.thread_index));
        }
        else {
            tx_backlog.queue.push_back(packet);
            tx_backlog_mask_ |= 1ull << packet.dst_chid.thread_index;
        }

        assert(tx_event_mask_ || tx_backlog_mask_);
        tx_event_.set();

        metrics_.route_count += 1;
    }

    void Router::deliver(const Packet& packet) {
        assert(thread_index_ == packet.dst_chid.thread_index);

        if (ChannelState* state = get_state(packet.dst_chid); LIKELY(state)) {
            state->rx_queue.event.set();
            state->rx_queue.queue.push_back(packet);
        }
        else {
            // The channel was destroyed.
        }

        metrics_.deliver_count += 1;
    }

    bool Router::forward(const Packet& packet) {
        const ThreadIndex dst_tidx = packet.dst_chid.thread_index;
        assert(thread_index_ != dst_tidx);

        SpscQueueProducer<Packet>& producer = get_tx_link(dst_tidx);
        if (!producer.insert(packet)) {
            // Link is at capacity. Try again later.
            metrics_.forward_failure_count += 1;
            return false;
        }

        tx_event_mask_ |= 1ull << packet.dst_chid.thread_index;

        metrics_.forward_success_count += 1;
        return true;
    }

    void Router::update_readiness() {
        tx_event_.set(tx_event_mask_ || tx_backlog_mask_);
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

    SpscQueueConsumer<Packet>& Router::get_rx_link(ThreadIndex tidx) {
        if (UNLIKELY(rx_links_.size() <= tidx)) {
            rx_links_.resize(tidx + 1);
            rx_links_[tidx] = SpscQueueConsumer<Packet>(fabric_->link(tidx, thread_index_).queue());
        }

        return rx_links_[tidx];
    }

    SpscQueueProducer<Packet>& Router::get_tx_link(ThreadIndex tidx) {
        if (UNLIKELY(tx_links_.size() <= tidx)) {
            tx_links_.resize(tidx + 1);
            tx_links_[tidx] = SpscQueueProducer<Packet>(fabric_->link(thread_index_, tidx).queue());
        }

        return tx_links_[tidx];
    }

    auto Router::get_tx_backlog(ThreadIndex tidx) -> PacketQueue& {
        if (UNLIKELY(tx_backlogs_.size() <= tidx)) {
            tx_backlogs_.resize(tidx + 1);
        }

        return tx_backlogs_[tidx];
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
