#include "slag/postal/domain.h"
#include <cassert>

namespace slag::postal {

    static       Domain* empire_instance = nullptr;
    static       Domain* nation_instance = nullptr;
    thread_local Domain* region_instance = nullptr;

    // Returns a pointer to the instance pointer of this type.
    Domain** domain_instance_pointer(DomainType type) {
        switch (type) {
            case DomainType::EMPIRE: return &empire_instance;
            case DomainType::NATION: return &nation_instance;
            case DomainType::REGION: return &region_instance;
        }

        abort();
    }

    Empire& empire() {
        assert(empire_instance);
        return static_cast<Empire&>(*empire_instance);
    }

    Nation& nation() {
        assert(nation_instance);
        return static_cast<Nation&>(*nation_instance);
    }

    Region& region() {
        assert(region_instance);
        return static_cast<Region&>(*region_instance);
    }

    Domain::Domain(DomainType type, size_t index)
        : type_{type}
        , index_{index}
    {
        Domain** instance = domain_instance_pointer(type_);
        assert(!*instance);
        *instance = this;
    }

    Domain::~Domain() {
        Domain** instance = domain_instance_pointer(type_);
        assert(*instance == this);
        *instance = nullptr;
    }

    DomainType Domain::type() const {
        return type_;
    }

    size_t Domain::index() const {
        return index_;
    }

    Empire::Empire(const Config& config)
        : Domain{DomainType::EMPIRE, config.index}
        , config_{config}
    {
    }

    auto Empire::config() const -> const Config& {
        return config_;
    }

    Nation::Nation(const Config& config)
        : Domain{DomainType::NATION, config.index}
        , empire_{empire()}
        , config_{config}
    {
        size_t region_count = config_.region_count;

        regions_.resize(region_count);
        for (auto&& region: regions_) {
            region = nullptr;
        }

        // O(n^2) routes since regions within a nation are fully connected
        parcel_queues_.resize(region_count * region_count);
        for (auto&& parcel_queue: parcel_queues_) {
            parcel_queue = std::make_unique<ParcelQueue>(
                config_.parcel_queue_capacity
            );
        }
    }

    auto Nation::config() const -> const Config& {
        return config_;
    }

    auto Nation::parcel_queue(PostArea to, PostArea from) -> ParcelQueue& {
        assert(to.nation == from.nation);
        assert(to.region != from.region);

        size_t index = (to.region * config_.region_count) + from.region;
        assert(index < parcel_queues_.size());
        return *parcel_queues_[index];
    }

    void Nation::attach(Region& region) {
        regions_[region.index()] = &region;
    }

    void Nation::detach(Region& region) {
        regions_[region.index()] = nullptr;
    }

    Region::Region(const Config& config)
        : Domain{DomainType::REGION, config.index}
        , nation_{nation()}
        , config_{config}
        , season_{Season::WINTER}
        , census_cursor_{nullptr}
        , post_office_{*this}
    {
        make_history();

        attach_parcel_queues();
    }

    Region::~Region() {
        detach_parcel_queues();
    }

    auto Region::census() const -> const Census& {
        return *census_cursor_;
    }

    auto Region::census(Season season) const -> const Census& {
        return history_[to_index(season)];
    }

    auto Region::imports() -> std::span<ParcelQueueConsumer> {
        return {
            imports_.data(),
            imports_.size(),
        };
    }

    auto Region::exports() -> std::span<ParcelQueueProducer> {
        return {
            exports_.data(),
            exports_.size(),
        };
    }

    PostArea Region::post_area() const {
        return make_post_area(config_.index);
    }

    auto Region::census_cursor() -> Census** {
        return &census_cursor_;
    }

    void Region::enter_season(Season season) {
        assert(next(season_) == season);

        season_        = season;
        census_cursor_ = &history_[to_index(season)];
    }

    void Region::leave_season(Season season) {
        assert(season_ == season);

        survey_parcel_queues();

        census_cursor_ = nullptr;
    }

    PostArea Region::make_post_area(size_t region_index) const {
        return {
            .nation = static_cast<uint16_t>(nation_.index()),
            .region = static_cast<uint16_t>(region_index),
        };
    }

    void Region::make_history() {
        size_t buffer_count = nation_.config().buffer_count;
        size_t region_count = nation_.config().region_count;

        for (Season season: seasons()) {
            Census& census = history_[to_index(season)];

            census.buffer_ownership_changes.grow_size_bits(buffer_count);
            census.buffer_reference_changes.grow_size_bits(buffer_count);

            census.import_sequences.resize(region_count);
            census.export_sequences.resize(region_count);
        }
    }

    void Region::attach_parcel_queues() {
        size_t region_count = nation_.config().region_count;

        imports_.resize(region_count);
        exports_.resize(region_count);

        for (size_t region_index = 0; region_index < region_count; ++region_index) {
            PostArea self = make_post_area(config_.index);
            PostArea peer = make_post_area(region_index);

            if (self.region == peer.region) {
                // this would be silly
            }
            else {
                imports_[region_index] = ParcelQueueConsumer(
                    nation_.parcel_queue(self, peer) // self <- peer
                );
                exports_[region_index] = ParcelQueueProducer(
                    nation_.parcel_queue(peer, self) // self -> peer
                );
            }
        }
    }

    void Region::detach_parcel_queues() {
        for (auto&& consumer: imports_) {
            consumer.reset();
        }

        for (auto&& producer: exports_) {
            producer.reset();
        }
    }

    void Region::survey_parcel_queues() {
        for (size_t import_index = 0; import_index < imports_.size(); ++import_index) {
            if (import_index == index()) {
                census_cursor_->import_sequences[import_index] = 0;
                continue;
            }

            census_cursor_->import_sequences[import_index] = imports_[import_index].flush();
        }

        for (size_t export_index = 0; export_index < exports_.size(); ++export_index) {
            if (export_index == index()) {
                census_cursor_->export_sequences[export_index] = 0;
                continue;
            }

            census_cursor_->export_sequences[export_index] = exports_[export_index].flush();
        }
    }

}
