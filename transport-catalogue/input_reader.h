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
  InputReader(TransportCatalogue &into) : storage_{into} {};

  template <class HandlerType>
  void ParseCommand(HandlerType type, std::string_view object_name,
                    std::string_view data) {
    auto handler = parse_handlers_.at(type);
    (*this.*handler)(object_name, data);
  }

  void InsertAllIntoStorage();

private:
  TransportCatalogue &storage_;

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
      stop_insert,
      slink_insert,
      bus_insert,
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
          {InputQueue::ActiveType::stop_insert, &InputReader::InsertStop},
          {InputQueue::ActiveType::slink_insert, &InputReader::InsertStopLinks},
          {InputQueue::ActiveType::bus_insert, &InputReader::InsertBus},
      };

  void ParseStopInfo(std::string_view object_name, std::string_view data);

  void InsertStop(const InputQueue &stop);

  void InsertStopLinks(const InputQueue &stop_links);

  void ParseBusInfo(std::string_view object_name, std::string_view data);

  void InsertBus(const InputQueue &bus);

  void FlushSavedDate();
};