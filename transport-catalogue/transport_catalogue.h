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
#include "serialization.h"


namespace core {
//! Objects of this type act as stops and routes database
class TransportCatalogue {
public:
  TransportCatalogue() = default;

  void ImportDataBase(const serialization::TrCatalogue &sr_catalogue);

  void ExportDataBase(serialization::Serializer &sr);

  void AddStop(const input_info::Stop &new_stop);

  void AddStopLinks(const input_info::StopLink &new_links);

  void AddBus(const input_info::Bus &new_bus);

  const data::BusStats *GetBusInfo(std::string_view bus_name) const;

  const data::StopStats *GetStopInfo(std::string_view stop_name) const;

  const data::BusStorage &GetBusStatsMap() const;

  std::vector<const data::Stop *> GetAllStops() const;

  std::optional<double> GetStopsRealDist(std::string_view from,
                                         std::string_view to) const;

private:
  std::deque<data::Stop> stops_;
  std::deque<data::Bus> buses_;
  data::BusStorage busname_to_bus_stats_;
  data::StopStorage stopname_to_stop_stats_;

  static double ComputeStopsDirectDist(const data::StopStats &from,
                                       const data::StopStats &to);

  static double GetStopsRealDist(const data::StopStats &from,
                                 const data::StopStats &to);

  static size_t GetBusTotalStopsAmount(const data::Bus &bus);
};

} // namespace data