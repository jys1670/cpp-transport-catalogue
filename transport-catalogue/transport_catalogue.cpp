#include "transport_catalogue.h"
#include "domain.h"

namespace core {
void TransportCatalogue::ImportDataBase(
    const serialization::TrCatalogue &sr_catalogue) {
  std::unordered_map<size_t, data::Stop *> id_to_stop_ptr;
  std::unordered_map<size_t, data::Bus *> id_to_bus_ptr;

  // Clear previous data
  stops_.clear();
  buses_.clear();
  busname_to_bus_stats_.clear();
  stopname_to_stop_stats_.clear();

  // list of stops
  size_t counter{0};
  for (const auto &sr_stop : sr_catalogue.stops()) {
    id_to_stop_ptr[counter++] = &stops_.emplace_back(
        data::Stop()
            .SetStopName(sr_stop.name())
            .SetCoordinates(
                {sr_stop.coordinates().lat(), sr_stop.coordinates().lng()}));
  }

  // list of buses
  counter = 0;
  for (const auto &sr_bus : sr_catalogue.buses()) {
    auto &bus = buses_.emplace_back(data::Bus()
                                        .SetBusName(sr_bus.name())
                                        .SetCircular(sr_bus.is_circular()));
    for (const auto &stop : sr_bus.stop_indexes()) {
      bus.AddStop(id_to_stop_ptr.at(stop));
    }
    id_to_bus_ptr[counter++] = &bus;
  }

  // stop stats
  for (const auto &sr_stop_stats : sr_catalogue.stopname_to_stop_stats()) {
    const auto stop_ptr = id_to_stop_ptr.at(sr_stop_stats.stop_index());
    auto &stop_stats =
        stopname_to_stop_stats_[stop_ptr->name].SetStop(stop_ptr);

    // linked buses
    for (const auto &linked_bus_index : sr_stop_stats.linked_buses_indexes()) {
      const auto linked_bus_ptr = id_to_bus_ptr.at(linked_bus_index);
      stop_stats.AddLinkedBus(linked_bus_ptr);
    }

    // linked stops
    for (int i{0}; i < sr_stop_stats.linked_stops_indexes_size(); ++i) {
      const auto linked_stop_ptr =
          id_to_stop_ptr.at(sr_stop_stats.linked_stops_indexes(i));
      const auto linked_stop_dist = sr_stop_stats.linked_stops_distances(i);
      stop_stats.SetLinkedStopDistance(linked_stop_ptr, linked_stop_dist);
    }
  }

  // bus stats
  for (const auto &sr_bus_stats : sr_catalogue.busname_to_bus_stats()) {
    const auto bus_ptr = id_to_bus_ptr.at(sr_bus_stats.bus_index());
    auto &bus_stats = busname_to_bus_stats_[bus_ptr->name]
                          .SetBus(bus_ptr)
                          .SetTotalStops(sr_bus_stats.total_stops())
                          .SetDirectLength(sr_bus_stats.direct_length())
                          .SetRealLength(sr_bus_stats.real_length());

    // unique stops
    std::unordered_set<data::Stop *> uniq_stops;
    for (const auto &uniq_stop : sr_bus_stats.uniq_stops_indexes()) {
      uniq_stops.insert(id_to_stop_ptr.at(uniq_stop));
    }
    bus_stats.SetUniqueStops(std::move(uniq_stops));
  }
}

void TransportCatalogue::ExportDataBase(serialization::Serializer &sr) {
  sr.SerializeStops(stops_);
  sr.SerializeBuses(buses_);
  sr.SerializeStopStats(stopname_to_stop_stats_);
  sr.SerializeBusStats(busname_to_bus_stats_);
}

void TransportCatalogue::AddStop(const input_info::Stop &new_stop) {
  auto &stop = stops_.emplace_back(
      data::Stop().SetStopName(new_stop.name).SetCoordinates(new_stop.pos));
  /* Linked buses and stops are initialized empty, since not every stop
   * was added to storage at this point of time, can't generate pointers to
   * structures
   */
  stopname_to_stop_stats_[stop.name] = data::StopStats().SetStop(&stop);
}

void TransportCatalogue::AddBus(const input_info::Bus &new_bus) {
  double total_dir_dist{}, total_real_dist{};
  std::unordered_set<data::Stop *> uniq_stops{};

  auto &bus = buses_.emplace_back(
      data::Bus().SetBusName(new_bus.name).SetCircular(new_bus.is_circular));

  if (!new_bus.stops.empty()) {
    auto first_stop_name = new_bus.stops.front();
    stopname_to_stop_stats_.at(first_stop_name).AddLinkedBus(&bus);

    data::Stop *first_stop_ptr =
        stopname_to_stop_stats_.at(first_stop_name).stop_ptr;
    bus.AddStop(first_stop_ptr);
    uniq_stops.insert(first_stop_ptr);

    for (auto it = next(new_bus.stops.begin()), end = new_bus.stops.end();
         it != end; ++it) {
      data::StopStats &prev_stop = stopname_to_stop_stats_.at(*prev(it));
      data::StopStats &curr_stop = stopname_to_stop_stats_.at(*it);
      double direct_dist = ComputeStopsDirectDist(prev_stop, curr_stop);
      double real_dist = GetStopsRealDist(prev_stop, curr_stop);

      bus.AddStop(curr_stop.stop_ptr);
      uniq_stops.insert(curr_stop.stop_ptr);
      curr_stop.AddLinkedBus(&bus);

      if (!new_bus.is_circular) {
        direct_dist *= 2;
        real_dist += GetStopsRealDist(curr_stop, prev_stop);
      }

      total_dir_dist += direct_dist;
      total_real_dist += real_dist;
    }
  }
  busname_to_bus_stats_[bus.name] =
      data::BusStats()
          .SetBus(&bus)
          .SetTotalStops(GetBusTotalStopsAmount(bus))
          .SetUniqueStops(std::move(uniq_stops))
          .SetDirectLength(total_dir_dist)
          .SetRealLength(total_real_dist);
}

void TransportCatalogue::AddStopLinks(const input_info::StopLink &new_links) {
  auto &from_stop = stopname_to_stop_stats_.at(new_links.stop_name);
  for (auto [stop_name, dist] : new_links.neighbours) {
    data::Stop *to_stop = stopname_to_stop_stats_.at(stop_name).stop_ptr;
    from_stop.SetLinkedStopDistance(to_stop, dist);
  }
}

const data::BusStats *
TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
  if (busname_to_bus_stats_.count(bus_name)) {
    return &busname_to_bus_stats_.at(bus_name);
  }
  return nullptr;
}

const data::StopStats *
TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
  if (stopname_to_stop_stats_.count(stop_name)) {
    return &stopname_to_stop_stats_.at(stop_name);
  }
  return nullptr;
}

double TransportCatalogue::ComputeStopsDirectDist(const data::StopStats &from,
                                                  const data::StopStats &to) {
  return ComputeDistance(from.stop_ptr->pos, to.stop_ptr->pos);
}

double TransportCatalogue::GetStopsRealDist(const data::StopStats &from,
                                            const data::StopStats &to) {
  if (from.linked_stops.count(to.stop_ptr)) {
    return from.linked_stops.at(to.stop_ptr);
  }
  return to.linked_stops.at(from.stop_ptr);
}

size_t TransportCatalogue::GetBusTotalStopsAmount(const data::Bus &bus) {
  if (bus.stops.empty())
    return 0;
  return bus.is_circular ? bus.stops.size() : bus.stops.size() * 2 - 1;
}

std::optional<double>
TransportCatalogue::GetStopsRealDist(const std::string_view from,
                                     const std::string_view to) const {
  if (stopname_to_stop_stats_.count(from) &&
      stopname_to_stop_stats_.count(to)) {
    auto &stop1 = stopname_to_stop_stats_.at(from);
    auto &stop2 = stopname_to_stop_stats_.at(to);
    if (stop1.linked_stops.count(stop2.stop_ptr)) {
      return stop1.linked_stops.at(stop2.stop_ptr);
    }
    if (stop2.linked_stops.count(stop1.stop_ptr)) {
      return stop2.linked_stops.at(stop1.stop_ptr);
    }
  }
  return std::nullopt;
}

const data::BusStorage &TransportCatalogue::GetBusStatsMap() const {
  return busname_to_bus_stats_;
}

std::vector<const data::Stop *> TransportCatalogue::GetAllStops() const {
  std::vector<const data::Stop *> result{};
  for (auto &stop : stops_) {
    result.emplace_back(&stop);
  }
  return result;
}
} // namespace core