/*!
 * \file transport_catalogue.h
 * \brief Stops and routes database
 */

#pragma once
#include <deque>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"

class TransportCatalogue {
public:
  void AddStop(const InputInfo::Stop &new_stop);

  void AddStopLinks(const InputInfo::StopLink &new_links);

  void AddBus(const InputInfo::Bus &new_bus);

  const DataStorage::BusStats *GetBusInfo(std::string_view bus_name) const;

  const DataStorage::StopStats *GetStopInfo(std::string_view stop_name) const;

  const DataStorage::StopStorage &GetStopStatsMap() const;

  const DataStorage::BusStorage &GetBusStatsMap() const;

  std::vector<const DataStorage::Stop *> GetAllStops() const;

  std::optional<double> GetStopsRealDist(std::string_view from,
                                         std::string_view to) const;

private:
  std::deque<DataStorage::Stop> stops_;
  std::deque<DataStorage::Bus> buses_;
  DataStorage::BusStorage busname_to_bus_stats_;
  DataStorage::StopStorage stopname_to_stop_stats_;


  static double ComputeStopsDirectDist(const DataStorage::StopStats &from,
                                       const DataStorage::StopStats &to);

  static double GetStopsRealDist(const DataStorage::StopStats &from,
                                 const DataStorage::StopStats &to);

  static size_t GetBusTotalStopsAmount(const DataStorage::Bus &bus);
};
