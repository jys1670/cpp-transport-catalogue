#include "request_handler.h"
#include "json_builder.h"

void RequestHandler::InsertIntoQueue(ReqsQueue &&elem) {
  reqs_queue_.emplace_back(elem);
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::PrintBusStats &req) {
  auto bus_info = parent_.catalogue_.GetBusInfo(req.bus_name);
  if (bus_info) {
    arr_.emplace_back(
        json::Builder()
            .StartDict()
            .Key("curvature")
            .Value(bus_info->real_length / bus_info->direct_lenght)
            .Key("request_id")
            .Value(req.id)
            .Key("route_length")
            .Value(bus_info->real_length)
            .Key("stop_count")
            .Value(static_cast<int>(bus_info->total_stops))
            .Key("unique_stop_count")
            .Value(static_cast<int>(bus_info->unique_stops.size()))
            .EndDict()
            .Build());
  } else {
    arr_.emplace_back(json::Builder()
                          .StartDict()
                          .Key("request_id")
                          .Value(req.id)
                          .Key("error_message")
                          .Value("not found")
                          .EndDict()
                          .Build());
  }
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::PrintStopStats &req) {
  auto stop_info = parent_.catalogue_.GetStopInfo(req.stop_name);
  if (stop_info) {
    if (stop_info->linked_buses.empty()) {
      arr_.emplace_back(json::Builder()
                            .StartDict()
                            .Key("buses")
                            .StartArray()
                            .EndArray()
                            .Key("request_id")
                            .Value(req.id)
                            .EndDict()
                            .Build());
    } else {
      std::vector<DataStorage::Bus *> pbus_vec(stop_info->linked_buses.begin(),
                                               stop_info->linked_buses.end());
      std::sort(pbus_vec.begin(), pbus_vec.end(),
                [](auto lhs, auto rhs) { return lhs->name < rhs->name; });
      json::Array buses;
      for (auto bus_ptr : pbus_vec) {
        buses.emplace_back(bus_ptr->name);
      }
      arr_.emplace_back(json::Builder()
                            .StartDict()
                            .Key("buses")
                            .Value(std::move(buses))
                            .Key("request_id")
                            .Value(req.id)
                            .EndDict()
                            .Build());
    }
  } else {
    arr_.emplace_back(json::Builder()
                          .StartDict()
                          .Key("request_id")
                          .Value(req.id)
                          .Key("error_message")
                          .Value("not found")
                          .EndDict()
                          .Build());
  }
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::UpdateMapRenderSettings &req) {
  parent_.renderer_.LoadSettings(*req.settings);
}

void RequestHandler::JsonPrint::operator()(
    const RequestHandler::RequestTypes::PrintMap &req) {
  arr_.emplace_back(
      json::Builder()
          .StartDict()
          .Key("map")
          .Value(parent_.renderer_.RenderMap(parent_.GetCatalogueData()))
          .Key("request_id")
          .Value(req.id)
          .EndDict()
          .Build());
}

void RequestHandler::JsonPrint::operator()(
    const RequestTypes::UpdateRoutingSettings &req) {
  parent_.trouter_.LoadSettings(*req.settings);
}

void RequestHandler::JsonPrint::operator()(const RequestTypes::Route &req) {
  auto answer = parent_.trouter_.FindFastestRoute(req.from, req.to);
  json::Array items{};
  if (answer) {
    for (RouteAnswer::Item &item : answer->items) {
      if (auto b_ptr = std::get_if<RouteAnswer::Bus>(&item)) {
        items.emplace_back(json::Builder()
                               .StartDict()
                               .Key("bus")
                               .Value(std::string{b_ptr->bus->name})
                               .Key("span_count")
                               .Value(static_cast<int>(b_ptr->span_count))
                               .Key("time")
                               .Value(b_ptr->time)
                               .Key("type")
                               .Value("Bus")
                               .EndDict()
                               .Build());
      } else if (auto w_ptr = std::get_if<RouteAnswer::Wait>(&item)) {
        items.emplace_back(json::Builder()
                               .StartDict()
                               .Key("stop_name")
                               .Value(std::string{w_ptr->stop->name})
                               .Key("time")
                               .Value(w_ptr->time)
                               .Key("type")
                               .Value("Wait")
                               .EndDict()
                               .Build());
      }
    }
    arr_.emplace_back(json::Builder()
                          .StartDict()
                          .Key("request_id")
                          .Value(req.id)
                          .Key("total_time")
                          .Value(answer->total_time)
                          .Key("items")
                          .Value(std::move(items))
                          .EndDict()
                          .Build());
  } else {
    arr_.emplace_back(json::Builder()
                          .StartDict()
                          .Key("request_id")
                          .Value(req.id)
                          .Key("error_message")
                          .Value("not found")
                          .EndDict()
                          .Build());
  }
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
