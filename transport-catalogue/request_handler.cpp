#include "request_handler.h"

void InputReader::ParseCommand(const json::Node &node) {
  json_parser_(node.AsMap().at("type").AsString(), node);
}

void InputReader::JsonParse::operator()(std::string_view type,
                                        const json::Node &node) {
  if (type == "Bus") {
    Bus(node);
    return;
  }
  if (type == "Stop") {
    Stop(node);
    return;
  }
}

void InputReader::JsonParse::Bus(const json::Node &node) {
  const auto &bus_map = node.AsMap();
  std::string_view bus_name = bus_map.at("name").AsString();
  InputInfo::Bus new_bus{bus_name, {}, bus_map.at("is_roundtrip").AsBool()};
  for (const auto &name : bus_map.at("stops").AsArray()) {
    new_bus.stops.emplace_back(name.AsString());
  }
  auto bus_ptr = &parent_.buses_input_queue_.emplace_back(std::move(new_bus));
  parent_.input_queue_.emplace_back(RequestTypes::BusInsert{bus_ptr});
}

void InputReader::JsonParse::Stop(const json::Node &node) {
  const auto &stop_map = node.AsMap();
  std::string_view stop_name = stop_map.at("name").AsString();
  InputInfo::Stop new_stop = {stop_name, {stop_map.at("latitude").AsDouble(), stop_map.at("longitude").AsDouble()}};
  InputInfo::StopLink new_stoplink = {stop_name, {}};
  for (const auto& [name, dist] : stop_map.at("road_distances").AsMap()) {
    new_stoplink.neighbours.emplace_back(name, dist.AsDouble());
  }
  auto stop_ptr = &parent_.stops_input_queue_.emplace_back(new_stop);
  parent_.input_queue_.emplace_back(RequestTypes::StopInsert{stop_ptr});
  auto stoplink_ptr = &parent_.stoplinks_input_queue_.emplace_back(new_stoplink);
  parent_.input_queue_.emplace_back(RequestTypes::StopLinkInsert {stoplink_ptr});
}

void InputReader::CatalogueInserter::operator()(
    const InputReader::RequestTypes::StopInsert &req) {
  parent_.storage_.AddStop(*req.stop_ptr);
}

void InputReader::CatalogueInserter::operator()(
    const InputReader::RequestTypes::StopLinkInsert &req) {
  parent_.storage_.AddStopLinks(*req.stoplink_ptr);
}

void InputReader::CatalogueInserter::operator()(
    const InputReader::RequestTypes::BusInsert &req) {
  parent_.storage_.AddBus(*req.bus_ptr);
}

void InputReader::InsertAllIntoStorage() {
  std::sort(input_queue_.begin(), input_queue_.end(),
            [](InputQueue &lhs, InputQueue &rhs) {
              return lhs.index() < rhs.index();
            });
  for (auto &elem : input_queue_) {
    std::visit(catalogue_inserter_, elem);
  }
  Clear();
}

void InputReader::Clear() {
  input_queue_.clear();
  stops_input_queue_.clear();
  stoplinks_input_queue_.clear();
  buses_input_queue_.clear();
}

void StatReader::ParseCommand(const json::Node &node) {
  json_parser_(node.AsMap().at("type").AsString(), node);
}

void StatReader::JsonPrint::operator()(
    const StatReader::RequestTypes::BusStatsPrint &req) {
  auto bus_info = parent_.storage_.GetBusInfo(req.bus_name);
  if (bus_info) {
    arr_.emplace_back(json::Node {
        json::Dict{
            {"curvature", bus_info->real_length / bus_info->direct_lenght},
            {"request_id", req.id},
            {"route_length", bus_info->real_length},
            {"stop_count", int(bus_info->total_stops)},
            {"unique_stop_count", int(bus_info->unique_stops.size())},
        },
    });
  } else {
    arr_.emplace_back(json::Node {
        json::Dict{
            {"request_id", req.id},
            {"error_message", "not found"},
        },
    });
  }
}

void StatReader::JsonPrint::operator()(
    const StatReader::RequestTypes::StopStatsPrint &req) {
  auto stop_info = parent_.storage_.GetStopInfo(req.stop_name);
  if (stop_info) {
    if (stop_info->linked_buses.empty()) {
      arr_.emplace_back(json::Node {
          json::Dict {
              {"buses", json::Array {}},
              {"request_id", req.id},
          }
      });
    } else {
      std::vector<DataStorage::Bus *> tmp(stop_info->linked_buses.begin(),
                                          stop_info->linked_buses.end());
      std::sort(tmp.begin(), tmp.end(),
                [](auto lhs, auto rhs) { return lhs->name < rhs->name; });
      json::Array buses;
      for (auto bus_ptr : tmp) {
        buses.push_back(bus_ptr->name);
      }
      arr_.emplace_back(json::Node {
          json::Dict {
              {"buses", std::move(buses)},
              {"request_id", req.id},
          }
      });
    }
  } else {
    arr_.emplace_back(json::Node {
        json::Dict {
            {"request_id", req.id},
            {"error_message", "not found"},
        }
    });
  }
}

void StatReader::JsonPrint::operator()(
    const StatReader::RequestTypes::MapPrint &req) {
  arr_.emplace_back(json::Node {
      json::Dict{
          {"map", json::Node(std::string{req.data})},
          {"request_id", req.id},
      },
  });
}

void StatReader::JsonParse::operator()(std::string_view type,
                                       const json::Node &node) {
  if (type == "Bus") {
    Bus(node);
    return;
  }
  if (type == "Stop") {
    Stop(node);
    return;
  }
  if (type == "Map") {
    Map(node);
    return;
  }
}

void StatReader::JsonParse::Bus(const json::Node &node) {
  const auto &bus_map = node.AsMap();
  parent_.stat_queue_.emplace_back(RequestTypes::BusStatsPrint{bus_map.at("id").AsInt(), bus_map.at("name").AsString()});
}

void StatReader::JsonParse::Stop(const json::Node &node) {
  const auto &stop_map = node.AsMap();
  parent_.stat_queue_.emplace_back(RequestTypes::StopStatsPrint{stop_map.at("id").AsInt(), stop_map.at("name").AsString()});
}

void StatReader::JsonParse::Map(const json::Node &node) {
  const auto &map_map = node.AsMap();
  auto& map = parent_.maps_.emplace_back(parent_.renderer_.RenderMap(parent_.GetCatalogueData()));
  parent_.stat_queue_.emplace_back(RequestTypes::MapPrint{map_map.at("id").AsInt(), map});
}


void StatReader::PrintAllStats(OutputFormat::Json) {
  json::Array result;
  JsonPrint visitor {*this, result};
  for (const auto &elem : stat_queue_) {
    std::visit(visitor, elem);
  }
  json::Print(json::Document{result}, outstream_);
  Clear();

}

void StatReader::Clear() {
  stat_queue_.clear();
  maps_.clear();
}

DataStorage::RoutesData StatReader::GetCatalogueData() const {
  std::unordered_set<const DataStorage::Stop *> stops_set;
  std::vector<const DataStorage::BusStats *> buses;
  for (const auto & [ _, stats] : storage_.GetBusStatsMap())  {
    stops_set.insert(stats.unique_stops.begin(), stats.unique_stops.end());
    buses.push_back(&stats);
  }
  std::vector<const DataStorage::Stop *> stops {stops_set.begin(), stops_set.end()};
  return {std::move(stops), std::move(buses)};
}
