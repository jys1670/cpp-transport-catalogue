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
#include <cstddef>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "domain.h"

class InputReader {
public:
  InputReader(TransportCatalogue &into)
      : storage_{into} {};

  void ParseCommand(const json::Node &node);

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

  using InputQueue = std::variant<RequestTypes::StopInsert, RequestTypes::StopLinkInsert, RequestTypes::BusInsert>;

  struct JsonParse {
  public:
    explicit JsonParse(InputReader & parent): parent_{parent} {};
    void operator()(std::string_view type, const json::Node &node);
  private:
    InputReader &parent_;
    void Bus(const json::Node &node);
    void Stop(const json::Node &node);
  } json_parser_ {*this};

  struct CatalogueInserter {
  public:
    explicit CatalogueInserter(InputReader &parent): parent_{parent} {};
    void operator()(const RequestTypes::StopInsert &req);
    void operator()(const RequestTypes::StopLinkInsert &req);
    void operator()(const RequestTypes::BusInsert &req);
  private:
    InputReader &parent_;
  } catalogue_inserter_ {*this};

  std::vector<InputQueue> input_queue_;
  std::deque<InputInfo::Stop> stops_input_queue_;
  std::deque<InputInfo::StopLink> stoplinks_input_queue_;
  std::deque<InputInfo::Bus> buses_input_queue_;

  void Clear();
};

class StatReader {
public:
  StatReader(TransportCatalogue &source, std::ostream &output, MapRenderer &renderer)
      : storage_{source}, outstream_{output}, renderer_{renderer} {};

  DataStorage::RoutesData GetCatalogueData() const;

  void ParseCommand(const json::Node &node);

  void PrintAllStats(OutputFormat::Json);

private:

  TransportCatalogue &storage_;
  std::ostream &outstream_;
  MapRenderer &renderer_;

  struct RequestTypes {

    struct BusStatsPrint {
      int id;
      std::string_view bus_name;
    };

    struct StopStatsPrint {
      int id;
      std::string_view stop_name;
    };

    struct MapPrint {
      int id;
      std::string_view data;
    };
  };

  using StatQueue = std::variant<RequestTypes::BusStatsPrint, RequestTypes::StopStatsPrint, RequestTypes::MapPrint>;

  struct JsonPrint {
  public:
    explicit JsonPrint(StatReader& parent, json::Array &arr): parent_{parent}, arr_{arr} {};
    void operator()(const RequestTypes::BusStatsPrint &req);
    void operator()(const RequestTypes::StopStatsPrint &req);
    void operator()(const RequestTypes::MapPrint &req);
  private:
    StatReader &parent_;
    json::Array &arr_;
  };

  struct JsonParse {
  public:
    explicit JsonParse(StatReader& parent): parent_{parent} {};
    void operator()(std::string_view type, const json::Node &node);
  private:
    StatReader &parent_;
    void Bus(const json::Node &node);
    void Stop(const json::Node &node);
    void Map(const json::Node &node);
  } json_parser_ {*this};

  std::vector<StatQueue> stat_queue_;
  std::deque<std::string> maps_;

  void Clear();
};