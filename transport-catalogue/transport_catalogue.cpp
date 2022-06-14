#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const InputInfo::Stop &new_stop) {
  auto &stop = stops_.emplace_back(
      Stop{std::string{new_stop.name}, new_stop.latitude, new_stop.longitude});
  stopname_to_stop_stats_[stop.name] = {&stop, {}, {}};
}

void TransportCatalogue::AddBus(const InputInfo::Bus &new_bus) {
  double total_dir_dist{}, total_real_dist{};
  std::unordered_set<Stop *> uniq_stops;

  auto &bus = buses_.emplace_back(
      Bus{std::string{new_bus.name}, {}, new_bus.is_circular});

  // Insert first stop, cycle over pairs (first, second) , (second, third) etc
  auto first_stop_name = new_bus.stops.front();
  stopname_to_stop_stats_.at(first_stop_name).linked_buses.insert(&bus);

  Stop *first_stop_ptr = stopname_to_stop_stats_.at(first_stop_name).stop_ptr;
  bus.stops.emplace_back(first_stop_ptr);
  uniq_stops.insert(first_stop_ptr);

  for (auto it = new_bus.stops.begin() + 1, end = new_bus.stops.end();
       it != end; ++it) {
    StopStats &prev_stop = stopname_to_stop_stats_.at(*(it - 1));
    StopStats &curr_stop = stopname_to_stop_stats_.at(*it);
    double direct_dist = ComputeStopsDirectDist(prev_stop, curr_stop);
    double real_dist = GetStopsRealDist(prev_stop, curr_stop);

    bus.stops.emplace_back(curr_stop.stop_ptr); // insert second, third, .. stop
    uniq_stops.insert(curr_stop.stop_ptr);
    curr_stop.linked_buses.insert(&bus);
    if (!new_bus.is_circular) {
      direct_dist *= 2;
      real_dist += GetStopsRealDist(curr_stop, prev_stop);
    }

    total_dir_dist += direct_dist;
    total_real_dist += real_dist;
  }

  busname_to_bus_stats_[bus.name] = BusStats{
      &bus,
      GetBusTotalStopsAmount(bus),
      std::move(uniq_stops),
      total_dir_dist,
      total_real_dist,
  };
}

void TransportCatalogue::AddStopLinks(const InputInfo::StopLink &new_links) {
  auto &main_stop = stopname_to_stop_stats_.at(new_links.stop_name);
  for (auto [stop_name, dist] : new_links.neighbours) {
    Stop *curr_stop_ptr = stopname_to_stop_stats_.at(stop_name).stop_ptr;
    main_stop.linked_stops[curr_stop_ptr] = dist;
  }
}

const TransportCatalogue::BusStats *
TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
  if (busname_to_bus_stats_.count(bus_name)) {
    return &busname_to_bus_stats_.at(bus_name);
  }
  return nullptr;
}

const TransportCatalogue::StopStats *
TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
  if (stopname_to_stop_stats_.count(stop_name)) {
    return &stopname_to_stop_stats_.at(stop_name);
  }
  return nullptr;
}

double TransportCatalogue::ComputeStopsDirectDist(const StopStats &first,
                                                  const StopStats &second) {
  return ComputeDistance(
      {first.stop_ptr->latitude, first.stop_ptr->longitude},
      {second.stop_ptr->latitude, second.stop_ptr->longitude});
}

double TransportCatalogue::GetStopsRealDist(const StopStats &first,
                                            const StopStats &second) {
  if (first.linked_stops.count(second.stop_ptr)) {
    return first.linked_stops.at(second.stop_ptr);
  }
  return second.linked_stops.at(first.stop_ptr);
}

size_t TransportCatalogue::GetBusTotalStopsAmount(const Bus &bus) {
  return bus.is_circular ? bus.stops.size() : bus.stops.size() * 2 - 1;
}
