#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const InputInfo::Stop &new_stop) {
  auto &stop = stops_.emplace_back(DataStorage::Stop()
                                       .SetStopName(new_stop.name)
                                       .SetCoordinates(new_stop.pos));
  /* Linked buses and stops are initialized empty, since not every stop
   * was added to storage at this point of time, can't generate pointers to
   * structures
   */
  stopname_to_stop_stats_[stop.name] = DataStorage::StopStats().SetStop(&stop);
}

void TransportCatalogue::AddBus(const InputInfo::Bus &new_bus) {
  double total_dir_dist{}, total_real_dist{};
  std::unordered_set<DataStorage::Stop *> uniq_stops{};

  auto &bus = buses_.emplace_back(DataStorage::Bus()
                                      .SetBusName(new_bus.name)
                                      .SetCircular(new_bus.is_circular));

  if (!new_bus.stops.empty()) {
    auto first_stop_name = new_bus.stops.front();
    stopname_to_stop_stats_.at(first_stop_name).AddLinkedBus(&bus);

    DataStorage::Stop *first_stop_ptr =
        stopname_to_stop_stats_.at(first_stop_name).stop_ptr;
    bus.AddStop(first_stop_ptr);
    uniq_stops.insert(first_stop_ptr);

    for (auto it = next(new_bus.stops.begin()), end = new_bus.stops.end();
         it != end; ++it) {
      DataStorage::StopStats &prev_stop = stopname_to_stop_stats_.at(*prev(it));
      DataStorage::StopStats &curr_stop = stopname_to_stop_stats_.at(*it);
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
      DataStorage::BusStats()
          .SetBus(&bus)
          .SetTotalStops(GetBusTotalStopsAmount(bus))
          .SetUniqueStops(std::move(uniq_stops))
          .SetDirectLength(total_dir_dist)
          .SetRealLength(total_real_dist);
}

void TransportCatalogue::AddStopLinks(const InputInfo::StopLink &new_links) {
  auto &from_stop = stopname_to_stop_stats_.at(new_links.stop_name);
  for (auto [stop_name, dist] : new_links.neighbours) {
    DataStorage::Stop *to_stop = stopname_to_stop_stats_.at(stop_name).stop_ptr;
    from_stop.SetLinkedStopDistance(to_stop, dist);
  }
}

const DataStorage::BusStats *
TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
  if (busname_to_bus_stats_.count(bus_name)) {
    return &busname_to_bus_stats_.at(bus_name);
  }
  return nullptr;
}

const DataStorage::StopStats *
TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
  if (stopname_to_stop_stats_.count(stop_name)) {
    return &stopname_to_stop_stats_.at(stop_name);
  }
  return nullptr;
}

double
TransportCatalogue::ComputeStopsDirectDist(const DataStorage::StopStats &from,
                                           const DataStorage::StopStats &to) {
  return ComputeDistance(from.stop_ptr->pos, to.stop_ptr->pos);
}

double TransportCatalogue::GetStopsRealDist(const DataStorage::StopStats &from,
                                            const DataStorage::StopStats &to) {
  if (from.linked_stops.count(to.stop_ptr)) {
    return from.linked_stops.at(to.stop_ptr);
  }
  return to.linked_stops.at(from.stop_ptr);
}

size_t TransportCatalogue::GetBusTotalStopsAmount(const DataStorage::Bus &bus) {
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

const DataStorage::StopStorage &TransportCatalogue::GetStopStatsMap() const {
  return stopname_to_stop_stats_;
}

const DataStorage::BusStorage &TransportCatalogue::GetBusStatsMap() const {
  return busname_to_bus_stats_;
}

std::vector<const DataStorage::Stop *> TransportCatalogue::GetAllStops() const {
  std::vector<const DataStorage::Stop *> result{};
  for (auto &stop : stops_) {
    result.emplace_back(&stop);
  }
  return result;
}
