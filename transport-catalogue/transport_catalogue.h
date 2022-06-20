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
  enum class QTypeId {
    Stop,
    Bus,
  };
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
  auto NameToId(std::string_view name) const { return name_to_id_.at(name); }

private:
  std::vector<QTypeInfo> types_list_;
  std::unordered_map<std::string_view, QTypeId> name_to_id_;
  std::unordered_map<QTypeId, std::string_view> id_to_name_;
};

namespace InputInfo {
struct Stop {
  std::string_view name;
  Coordinates pos;
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

namespace DataStorage {
struct Stop {
  std::string name;
  Coordinates pos;
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
} // namespace DataStorage

class TransportCatalogue {
public:
  void AddStop(const InputInfo::Stop &new_stop);

  void AddStopLinks(const InputInfo::StopLink &new_links);

  void AddBus(const InputInfo::Bus &new_bus);

  const DataStorage::BusStats *GetBusInfo(std::string_view bus_name) const;

  const DataStorage::StopStats *GetStopInfo(std::string_view stop_name) const;

  double GetStopsDirectDistance(std::string_view from, std::string_view to);

private:
  std::deque<DataStorage::Stop> stops_;
  std::deque<DataStorage::Bus> buses_;
  std::unordered_map<std::string_view, DataStorage::BusStats>
      busname_to_bus_stats_;
  std::unordered_map<std::string_view, DataStorage::StopStats>
      stopname_to_stop_stats_;

  struct StopsStatsHasher {
    size_t operator()(const std::pair<DataStorage::Stop *, DataStorage::Stop *>
                          object) const {
      return (size_t)object.first + (size_t)object.second * 37;
    }
  };

  std::unordered_map<std::pair<DataStorage::Stop *, DataStorage::Stop *>,
                     double, StopsStatsHasher>
      direct_distances_;

  static double ComputeStopsDirectDist(const DataStorage::StopStats &from,
                                       const DataStorage::StopStats &to);

  static double GetStopsRealDist(const DataStorage::StopStats &from,
                                 const DataStorage::StopStats &to);

  static size_t GetBusTotalStopsAmount(const DataStorage::Bus &bus);
};
