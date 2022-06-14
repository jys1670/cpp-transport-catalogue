#pragma once
#include <deque>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

class QTypes {
public:
  enum class QTypeId { Stop, Bus };
  struct QTypeInfo {
    std::string name;
    QTypeId id;
  };
  QTypes(std::initializer_list<QTypeInfo> list) {
    types_list_.reserve(list.size());
    for (const auto &it : list) {
      QTypeInfo &curr_elem = types_list_.emplace_back(it);
      name_to_id_[curr_elem.name] = curr_elem.id;
      id_to_name_[curr_elem.id] = curr_elem.name;
    }
  }
  const auto &GetNamesToIds() const { return name_to_id_; }
  const auto &GetIdToNames() const { return id_to_name_; }

private:
  std::vector<QTypeInfo> types_list_;
  std::unordered_map<std::string_view, QTypeId> name_to_id_;
  std::unordered_map<QTypeId, std::string_view> id_to_name_;
};

namespace InputInfo {
struct Stop {
  std::string_view name;
  double latitude;
  double longitude;
};
struct StopLink {
  std::string_view stop_name;
  std::vector<std::pair<std::string_view, double>> neighbours;
};
struct Bus {
  std::string_view name;
  std::vector<std::string_view> stops;
  bool is_circular;
};
} // namespace InputInfo

class TransportCatalogue {
public:
  struct Stop {
    std::string name;
    double latitude;
    double longitude;
  };
  struct Bus {
    std::string name;
    std::vector<Stop *> stops;
    bool is_circular;
  };
  struct StopStats {
    Stop *stop_ptr;
    std::unordered_set<Bus *> linked_buses;
    std::unordered_map<Stop *, double> linked_stops;
  };
  struct BusStats {
    Bus *bus_ptr;
    size_t total_stops;
    std::unordered_set<Stop *> unique_stops;
    double direct_lenght;
    double real_length;
  };
  void AddStop(const InputInfo::Stop &new_stop);

  void AddStopLinks(const InputInfo::StopLink &new_links);

  void AddBus(const InputInfo::Bus &new_bus);

  const BusStats *GetBusInfo(std::string_view bus_name) const;

  const StopStats *GetStopInfo(std::string_view stop_name) const;

private:
  std::deque<Stop> stops_;
  std::deque<Bus> buses_;
  std::unordered_map<std::string_view, BusStats> busname_to_bus_stats_;
  std::unordered_map<std::string_view, StopStats> stopname_to_stop_stats_;

  static double ComputeStopsDirectDist(const StopStats &first,
                                       const StopStats &second);

  static double GetStopsRealDist(const StopStats &first,
                                 const StopStats &second);

  static size_t GetBusTotalStopsAmount(const Bus &bus);
};
