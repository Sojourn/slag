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

        // O(n^2) routes since regions within a nation are fully connected
        regional_routes_.resize(region_count * region_count);

        for (std::unique_ptr<RegionalRoute>& regional_route: regional_routes_) {
            regional_route = std::make_unique<RegionalRoute>(
                config_.region_route_capacity
            );
        }
    }

    auto Nation::config() const -> const Config& {
        return config_;
    }

    auto Nation::regional_route(PostArea to, PostArea from) -> RegionalRoute& {
        assert(to.nation == from.nation);
        assert(to.region != from.region);

        size_t index = (to.region * config_.region_count) + from.region;
        assert(index < regional_routes_.size());
        return *regional_routes_[index];
    }

    Region::Region(const Config& config)
        : Domain{DomainType::REGION, config.index}
        , nation_{nation()}
        , config_{config}
        , season_{Season::WINTER}
        , census_cursor_{nullptr}
    {
        setup_routes();
        setup_history();
    }

    auto Region::census() const -> const Census& {
        return *census_cursor_;
    }

    auto Region::census(Season season) const -> const Census& {
        return history_[to_index(season)];
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

        survey_imports();
        survey_exports();

        census_cursor_ = nullptr;
    }

    void Region::setup_routes() {
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
                imports_[region_index] = SpscQueueConsumer(
                    nation_.regional_route(self, peer) // self <- peer
                );
                exports_[region_index] = SpscQueueProducer(
                    nation_.regional_route(peer, self) // self -> peer
                );
            }
        }
    }

    void Region::setup_history() {
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

    void Region::survey_imports() {
        for (size_t import_index = 0; import_index < imports_.size(); ++import_index) {
            if (import_index == index()) {
                census_cursor_->import_sequences[import_index] = 0;
                continue;
            }

            census_cursor_->import_sequences[import_index] = imports_[import_index].flush();
        }
    }

    void Region::survey_exports() {
        for (size_t export_index = 0; export_index < exports_.size(); ++export_index) {
            if (export_index == index()) {
                census_cursor_->export_sequences[export_index] = 0;
                continue;
            }

            census_cursor_->export_sequences[export_index] = exports_[export_index].flush();
        }
    }

    PostArea Region::make_post_area(size_t region_index) const {
        return {
            .nation = static_cast<uint16_t>(nation_.index()),
            .region = static_cast<uint16_t>(region_index),
        };
    }

}
