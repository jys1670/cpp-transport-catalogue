#pragma once

#include <algorithm>
#include <cassert>
#include <charconv>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "stat_reader.h"
#include "transport_catalogue.h"

class InputReader {
public:
  InputReader(TransportCatalogue &into, QTypes &known_types)
      : storage_{into}, known_types_{known_types} {};

  void ParseCommand(std::string_view request) {
    size_t colon_pos = request.find_first_of(':');
    size_t space_pos = request.find_first_of(' ');
    std::string_view command_name = request.substr(0, space_pos);
    std::string_view object_name =
        request.substr(space_pos + 1, colon_pos - space_pos - 1);
    std::string_view data = request.substr(colon_pos + 2);
    auto handler = parse_handlers_.at(known_types_.NameToId(command_name));
    (*this.*handler)(object_name, data);
  }

  void InsertAllIntoStorage();

private:
  TransportCatalogue &storage_;
  QTypes &known_types_;

  struct RequestTypes {
    struct StopInsert {
      InputInfo::Stop *stop_ptr;
    };
    struct StopLinkInsert {
      InputInfo::StopLink *stoplink_ptr;
    };
    struct BusInsert {
      InputInfo::Bus *bus_ptr;
    };
  };

  struct InputQueue {
    enum ActiveType {
      StopInsert,
      SLinkInsert,
      BusInsert,
    } active_type;
    union Request {
      RequestTypes::StopInsert stop_input;
      RequestTypes::StopLinkInsert stoplink_input;
      RequestTypes::BusInsert bus_input;
    } data;
  };

  std::vector<InputQueue> input_queue_;
  std::deque<InputInfo::Stop> stops_input_queue_;
  std::deque<InputInfo::StopLink> stoplinks_input_queue_;
  std::deque<InputInfo::Bus> buses_input_queue_;

  using ParseHandler = void (InputReader::*)(std::string_view,
                                             std::string_view);
  const std::unordered_map<QTypes::QTypeId, ParseHandler> parse_handlers_{
      {QTypes::QTypeId::Stop, &InputReader::ParseStopInfo},
      {QTypes::QTypeId::Bus, &InputReader::ParseBusInfo},
  };

  using InsertHandler = void (InputReader::*)(const InputQueue &);
  const std::unordered_map<InputQueue::ActiveType, InsertHandler>
      insert_handlers_{
          {InputQueue::StopInsert, &InputReader::InsertStop},
          {InputQueue::SLinkInsert, &InputReader::InsertStopLinks},
          {InputQueue::BusInsert, &InputReader::InsertBus},
      };

  void ParseStopInfo(std::string_view object_name, std::string_view data);

  void InsertStop(const InputQueue &stop);

  void InsertStopLinks(const InputQueue &stop_links);

  void ParseBusInfo(std::string_view object_name, std::string_view data);

  void InsertBus(const InputQueue &bus);

  void Clear();
};