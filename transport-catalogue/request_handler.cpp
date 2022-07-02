#include "request_handler.h"

void RequestHandler::InsertIntoQueue(ReqsQueue &&elem) {
  reqs_queue_.emplace_back(elem);
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::PrintBusStats &req) {
  auto bus_info = parent_.catalogue_.GetBusInfo(req.bus_name);
  if (bus_info) {
    arr_.emplace_back(json::Node{
        json::Dict{
            {"curvature", bus_info->real_length / bus_info->direct_lenght},
            {"request_id", req.id},
            {"route_length", bus_info->real_length},
            {"stop_count", int(bus_info->total_stops)},
            {"unique_stop_count", int(bus_info->unique_stops.size())},
        },
    });
  } else {
    arr_.emplace_back(json::Node{
        json::Dict{
            {"request_id", req.id},
            {"error_message", "not found"},
        },
    });
  }
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::PrintStopStats &req) {
  auto stop_info = parent_.catalogue_.GetStopInfo(req.stop_name);
  if (stop_info) {
    if (stop_info->linked_buses.empty()) {
      arr_.emplace_back(json::Node{json::Dict{
          {"buses", json::Array{}},
          {"request_id", req.id},
      }});
    } else {
      std::vector<DataStorage::Bus *> pbus_vec(stop_info->linked_buses.begin(),
                                               stop_info->linked_buses.end());
      std::sort(pbus_vec.begin(), pbus_vec.end(),
                [](auto lhs, auto rhs) { return lhs->name < rhs->name; });
      json::Array buses;
      for (auto bus_ptr : pbus_vec) {
        buses.push_back(bus_ptr->name);
      }
      arr_.emplace_back(json::Node{json::Dict{
          {"buses", std::move(buses)},
          {"request_id", req.id},
      }});
    }
  } else {
    arr_.emplace_back(json::Node{json::Dict{
        {"request_id", req.id},
        {"error_message", "not found"},
    }});
  }
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::UpdateMapRenderSettings &req) {
  parent_.renderer_.LoadSettings(*req.settings);
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::PrintMap &req) {
  arr_.emplace_back(json::Node{
      json::Dict{
          {"map",
           json::Node(parent_.renderer_.RenderMap(parent_.GetCatalogueData()))},
          {"request_id", req.id},
      },
  });
}

void RequestHandler::ProcessAllRequests(OutputFormat::Json) {
  json::Array result;
  JsonPrint visitor{*this, result};
  for (const auto &elem : reqs_queue_) {
    std::visit(visitor, elem);
  }
  json::Print(json::Document{result}, outstream_);
  Clear();
}

void RequestHandler::Clear() { reqs_queue_.clear(); }

DataStorage::RoutesData RequestHandler::GetCatalogueData() const {
  std::unordered_set<const DataStorage::Stop *> stops_set;
  std::vector<const DataStorage::BusStats *> buses;
  for (const auto &[_, stats] : catalogue_.GetBusStatsMap()) {
    stops_set.insert(stats.unique_stops.begin(), stats.unique_stops.end());
    buses.push_back(&stats);
  }
  std::vector<const DataStorage::Stop *> stops{stops_set.begin(),
                                               stops_set.end()};
  return {std::move(stops), std::move(buses)};
}
