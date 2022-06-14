#pragma once
#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>
#include <string_view>
#include <vector>

template <typename OutputStream> class StatReader {
public:
  StatReader(TransportCatalogue &source, OutputStream &output)
      : storage_{source}, outstream_{output} {};

  template <class HandlerType>
  void ParseCommand(HandlerType type, std::string_view object_name);

  void PrintAllStats();

private:
  TransportCatalogue &storage_;
  OutputStream &outstream_;

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
      bus_stats_print,
      stop_stats_print,
    } active_type;
    union Request {
      typename RequestTypes::BusStatsPrint bus_stats_print;
      typename RequestTypes::StopStatsPrint stop_stats_print;
    } data;
  };

  std::vector<StatQueue> stat_queue;

  using ParseHandler = void (StatReader::*)(std::string_view);
  const std::unordered_map<QTypes::QTypeId, ParseHandler> parse_handlers_{
      {QTypes::QTypeId::Bus, &StatReader::ParseBusQuery},
      {QTypes::QTypeId::Stop, &StatReader::ParseStopQuery},
  };

  using PrintHandler = void (StatReader::*)(const StatQueue &);
  const std::unordered_map<typename StatQueue::ActiveType, PrintHandler>
      print_handlers_{
          {StatQueue::ActiveType::bus_stats_print, &StatReader::PrintBusQuery},
          {StatQueue::ActiveType::stop_stats_print,
           &StatReader::PrintStopQuery},
      };

  void ParseBusQuery(std::string_view bus_name);

  void PrintBusQuery(const StatQueue &elem);

  void ParseStopQuery(std::string_view stop_name);

  void PrintStopQuery(const StatQueue &elem);

  void FlushSavedDate();
};

template <typename OutputStream>
template <class HandlerType>
void StatReader<OutputStream>::ParseCommand(HandlerType type,
                                            std::string_view object_name) {
  auto handler = parse_handlers_.at(type);
  (*this.*handler)(object_name);
}

template <typename OutputStream>
void StatReader<OutputStream>::PrintAllStats() {
  for (const auto &elem : stat_queue) {
    auto handler = print_handlers_.at(elem.active_type);
    (*this.*handler)(elem);
  }
  FlushSavedDate();
}

template <typename OutputStream>
void StatReader<OutputStream>::ParseBusQuery(std::string_view bus_name) {
  typename StatQueue::Request data{};
  data.bus_stats_print.bus_name = bus_name;
  stat_queue.push_back({StatQueue::bus_stats_print, data});
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
  stat_queue.push_back({StatQueue::stop_stats_print, data});
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
      std::vector<TransportCatalogue::Bus *> tmp(
          stop_info->linked_buses.begin(), stop_info->linked_buses.end());
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

template <typename OutputStream>
void StatReader<OutputStream>::FlushSavedDate() {
  stat_queue.clear();
}
