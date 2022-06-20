#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const InputInfo::Stop &new_stop) {
  auto &stop = stops_.emplace_back(
      DataStorage::Stop{std::string{new_stop.name}, new_stop.pos});
  // Linked buses and linked stops are initialized empty, since not every stop
  // was added to storage at this point of time, can't generate pointers to structures
  stopname_to_stop_stats_[stop.name] = {&stop, {}, {}};
}

void TransportCatalogue::AddBus(const InputInfo::Bus &new_bus) {
  double total_dir_dist{}, total_real_dist{};
  std::unordered_set<DataStorage::Stop *> uniq_stops;

  auto &bus = buses_.emplace_back(
      DataStorage::Bus{std::string{new_bus.name}, {}, new_bus.is_circular});

  // Insert first stop, cycle over pairs (first, second) , (second, third) etc
  auto first_stop_name = new_bus.stops.front();
  stopname_to_stop_stats_.at(first_stop_name).linked_buses.insert(&bus);

  DataStorage::Stop *first_stop_ptr =
      stopname_to_stop_stats_.at(first_stop_name).stop_ptr;
  bus.stops.emplace_back(first_stop_ptr);
  uniq_stops.insert(first_stop_ptr);

  for (auto it = new_bus.stops.begin() + 1, end = new_bus.stops.end();
       it != end; ++it) {
    DataStorage::StopStats &prev_stop = stopname_to_stop_stats_.at(*(it - 1));
    DataStorage::StopStats &curr_stop = stopname_to_stop_stats_.at(*it);
    double direct_dist = ComputeStopsDirectDist(prev_stop, curr_stop);
    double real_dist = GetStopsRealDist(prev_stop, curr_stop);

    direct_distances_[{prev_stop.stop_ptr, curr_stop.stop_ptr}] = direct_dist;
    direct_distances_[{curr_stop.stop_ptr, prev_stop.stop_ptr}] = direct_dist;

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

  busname_to_bus_stats_[bus.name] = DataStorage::BusStats{
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
    DataStorage::Stop *curr_stop_ptr =
        stopname_to_stop_stats_.at(stop_name).stop_ptr;
    main_stop.linked_stops[curr_stop_ptr] = dist;
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
  return bus.is_circular ? bus.stops.size() : bus.stops.size() * 2 - 1;
}

double TransportCatalogue::GetStopsDirectDistance(std::string_view from,
                                                  std::string_view to) {
  if (stopname_to_stop_stats_.count(from) &&
      stopname_to_stop_stats_.count(to)) {
    DataStorage::StopStats &stop1 = stopname_to_stop_stats_.at(from);
    DataStorage::StopStats &stop2 = stopname_to_stop_stats_.at(to);
    return direct_distances_.at({stop1.stop_ptr, stop2.stop_ptr});
  }
  return -1; // makes no sense, error case
}
