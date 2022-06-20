#pragma once
#include <algorithm>
#include <iostream>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"

template <typename OutputStream> class StatReader {
public:
  StatReader(TransportCatalogue &source, OutputStream &output,
             QTypes &known_types)
      : storage_{source}, outstream_{output}, known_types_{known_types} {};

  void ParseCommand(std::string_view request);

  void PrintAllStats();

private:
  TransportCatalogue &storage_;
  OutputStream &outstream_;
  QTypes &known_types_;

  struct RequestTypes {
    struct BusStatsPrint {
      std::string_view bus_name;
    };
    struct StopStatsPrint {
      std::string_view stop_name;
    };
  };

  struct StatQueue {
    enum ActiveType {
      BusStatsPrint = 0,
      StopStatsPrint,
    } active_type;
    union Request {
      typename RequestTypes::BusStatsPrint bus_stats_print;
      typename RequestTypes::StopStatsPrint stop_stats_print;
    } data;
  };

  std::vector<StatQueue> stat_queue_;

  using ParseHandler = void (StatReader::*)(std::string_view);
  const std::unordered_map<QTypes::QTypeId, ParseHandler> parse_handlers_{
      {QTypes::QTypeId::Bus, &StatReader::ParseBusQuery},
      {QTypes::QTypeId::Stop, &StatReader::ParseStopQuery},
  };

  using PrintHandler = void (StatReader::*)(const StatQueue &);
  const std::unordered_map<typename StatQueue::ActiveType, PrintHandler>
      print_handlers_{
          {StatQueue::BusStatsPrint, &StatReader::PrintBusQuery},
          {StatQueue::StopStatsPrint, &StatReader::PrintStopQuery},
      };

  void ParseBusQuery(std::string_view bus_name);

  void PrintBusQuery(const StatQueue &elem);

  void ParseStopQuery(std::string_view stop_name);

  void PrintStopQuery(const StatQueue &elem);

  void Clear();
};

template <typename OutputStream>
void StatReader<OutputStream>::ParseCommand(std::string_view request) {
  auto delim_pos = request.find_first_of(' ');
  auto command_name = request.substr(0, delim_pos);
  auto object_name = request.substr(delim_pos + 1);
  auto handler = parse_handlers_.at(known_types_.NameToId(command_name));
  (*this.*handler)(object_name);
}

template <typename OutputStream>
void StatReader<OutputStream>::PrintAllStats() {
  for (const auto &elem : stat_queue_) {
    auto handler = print_handlers_.at(elem.active_type);
    (*this.*handler)(elem);
  }
  Clear();
}

template <typename OutputStream>
void StatReader<OutputStream>::ParseBusQuery(std::string_view bus_name) {
  typename StatQueue::Request data{};
  data.bus_stats_print.bus_name = bus_name;
  stat_queue_.push_back({StatQueue::BusStatsPrint, data});
}

template <typename OutputStream>
void StatReader<OutputStream>::PrintBusQuery(
    const StatReader::StatQueue &elem) {
  auto bus_name = elem.data.bus_stats_print.bus_name;
  auto bus_info = storage_.GetBusInfo(bus_name);
  if (bus_info) {
    outstream_ << "Bus " << bus_name << ": " << bus_info->total_stops
               << " stops on route, " << bus_info->unique_stops.size()
               << " unique stops, " << bus_info->real_length
               << " route length, "
               << bus_info->real_length / bus_info->direct_lenght
               << " curvature" << std::endl;
  } else {
    outstream_ << "Bus " << bus_name << ": not found" << std::endl;
  }
}

template <typename OutputStream>
void StatReader<OutputStream>::ParseStopQuery(std::string_view stop_name) {
  typename StatQueue::Request data{};
  data.stop_stats_print.stop_name = stop_name;
  stat_queue_.push_back({StatQueue::StopStatsPrint, data});
}

template <typename OutputStream>
void StatReader<OutputStream>::PrintStopQuery(
    const StatReader::StatQueue &elem) {
  auto stop_name = elem.data.stop_stats_print.stop_name;
  auto stop_info = storage_.GetStopInfo(stop_name);
  if (stop_info) {
    if (stop_info->linked_buses.empty()) {
      outstream_ << "Stop " << stop_name << ": no buses" << std::endl;
    } else {
      std::vector<DataStorage::Bus *> tmp(stop_info->linked_buses.begin(),
                                          stop_info->linked_buses.end());
      std::sort(tmp.begin(), tmp.end(),
                [](auto lhs, auto rhs) { return lhs->name < rhs->name; });
      outstream_ << "Stop " << stop_name << ": buses";
      for (auto bus_ptr : tmp) {
        outstream_ << " " << bus_ptr->name;
      }
      outstream_ << std::endl;
    }
  } else {
    outstream_ << "Stop " << stop_name << ": not found" << std::endl;
  }
}

template <typename OutputStream> void StatReader<OutputStream>::Clear() {
  stat_queue_.clear();
}
