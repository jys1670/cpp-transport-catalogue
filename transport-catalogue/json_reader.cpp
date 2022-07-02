#include "json_reader.h"

void JsonReader::ProcessInput(OutputFormat::Json) {
  const json::Document doc = json::Load(inputstream_);
  const auto &doc_map = doc.GetRoot().AsMap();
  for (const auto &node : doc_map.at("base_requests").AsArray()) {
    ParseSingleCommand(node);
  }
  InsertAllIntoCatalogue();
  req_handler_.InsertIntoQueue(
      RequestHandler::RequestTypes::UpdateMapRenderSettings{
          &doc_map.at("render_settings")});
  for (const auto &node : doc_map.at("stat_requests").AsArray()) {
    json_print_parser_(node.AsMap().at("type").AsString(), node);
  }
  req_handler_.ProcessAllRequests(OutputFormat::Json{});
}

void JsonReader::ParseSingleCommand(const json::Node &node) {
  json_input_parser_(node.AsMap().at("type").AsString(), node);
}

void JsonReader::JsonInputParse::operator()(std::string_view type,
                                            const json::Node &node) {
  auto handler = handlers_.at(type);
  (*this.*handler)(node);
}

void JsonReader::JsonInputParse::EnqueueBus(const json::Node &node) {
  const auto &bus_map = node.AsMap();
  std::string_view bus_name = bus_map.at("name").AsString();
  InputInfo::Bus new_bus{bus_name, {}, bus_map.at("is_roundtrip").AsBool()};
  for (const auto &name : bus_map.at("stops").AsArray()) {
    new_bus.stops.emplace_back(name.AsString());
  }
  auto bus_ptr = &parent_.buses_input_queue_.emplace_back(std::move(new_bus));
  parent_.input_queue_.emplace_back(RequestTypes::BusInsert{bus_ptr});
}

void JsonReader::JsonInputParse::EnqueueStop(const json::Node &node) {
  const auto &stop_map = node.AsMap();
  std::string_view stop_name = stop_map.at("name").AsString();
  InputInfo::Stop new_stop = {stop_name,
                              {stop_map.at("latitude").AsDouble(),
                               stop_map.at("longitude").AsDouble()}};
  InputInfo::StopLink new_stoplink = {stop_name, {}};
  for (const auto &[name, dist] : stop_map.at("road_distances").AsMap()) {
    new_stoplink.neighbours.emplace_back(name, dist.AsDouble());
  }
  auto stop_ptr = &parent_.stops_input_queue_.emplace_back(new_stop);
  parent_.input_queue_.emplace_back(RequestTypes::StopInsert{stop_ptr});
  auto stoplink_ptr =
      &parent_.stoplinks_input_queue_.emplace_back(new_stoplink);
  parent_.input_queue_.emplace_back(RequestTypes::StopLinkInsert{stoplink_ptr});
}

std::unordered_map<std::string_view, JsonReader::JsonInputParse::FunctionPtr>
    JsonReader::JsonInputParse::handlers_ = {
        {"Bus", &JsonReader::JsonInputParse::EnqueueBus},
        {"Stop", &JsonReader::JsonInputParse::EnqueueStop},
};

void JsonReader::JsonPrintParse::operator()(std::string_view type,
                                            const json::Node &node) {
  auto handler = handlers_.at(type);
  (*this.*handler)(node);
}

void JsonReader::JsonPrintParse::EnqueueBus(const json::Node &node) {
  const auto &bus_map = node.AsMap();
  parent_.InsertIntoQueue(RequestTypes::PrintBusStats{
      bus_map.at("id").AsInt(), bus_map.at("name").AsString()});
}

void JsonReader::JsonPrintParse::EnqueueStop(const json::Node &node) {
  const auto &stop_map = node.AsMap();
  parent_.InsertIntoQueue(RequestTypes::PrintStopStats{
      stop_map.at("id").AsInt(), stop_map.at("name").AsString()});
}

void JsonReader::JsonPrintParse::EnqueueMapDraw(const json::Node &node) {
  parent_.InsertIntoQueue(
      RequestTypes::PrintMap{node.AsMap().at("id").AsInt()});
}

std::unordered_map<std::string_view, JsonReader::JsonPrintParse::FunctionPtr>
    JsonReader::JsonPrintParse::handlers_ = {
        {"Bus", &JsonReader::JsonPrintParse::EnqueueBus},
        {"Stop", &JsonReader::JsonPrintParse::EnqueueStop},
        {"Map", &JsonReader::JsonPrintParse::EnqueueMapDraw},
};

void JsonReader::CatalogueInserter::operator()(
    const JsonReader::RequestTypes::StopInsert &req) {
  parent_.catalogue_.AddStop(*req.stop_ptr);
}

void JsonReader::CatalogueInserter::operator()(
    const JsonReader::RequestTypes::StopLinkInsert &req) {
  parent_.catalogue_.AddStopLinks(*req.stoplink_ptr);
}

void JsonReader::CatalogueInserter::operator()(
    const JsonReader::RequestTypes::BusInsert &req) {
  parent_.catalogue_.AddBus(*req.bus_ptr);
}

void JsonReader::InsertAllIntoCatalogue() {
  std::sort(input_queue_.begin(), input_queue_.end(),
            [](InputQueue &lhs, InputQueue &rhs) {
              return lhs.index() < rhs.index();
            });
  for (auto &elem : input_queue_) {
    std::visit(catalogue_inserter_, elem);
  }
  Clear();
}

void JsonReader::Clear() {
  input_queue_.clear();
  stops_input_queue_.clear();
  stoplinks_input_queue_.clear();
  buses_input_queue_.clear();
}