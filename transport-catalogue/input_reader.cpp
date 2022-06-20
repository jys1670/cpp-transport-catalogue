#include "input_reader.h"

void InputReader::InsertAllIntoStorage() {
  std::sort(input_queue_.begin(), input_queue_.end(),
            [](InputQueue &lhs, InputQueue &rhs) {
              return lhs.active_type < rhs.active_type;
            });
  for (auto &elem : input_queue_) {
    auto handler = insert_handlers_.at(elem.active_type);
    (*this.*handler)(elem);
  }
  Clear();
}

void InputReader::ParseStopInfo(std::string_view object_name,
                                std::string_view data) {
  // Only object name is know right now, coordinates must be parsed from data
  InputInfo::Stop new_stop = {object_name, {}};
  // Neighbours must be parsed from data
  InputInfo::StopLink new_stoplink = {object_name, {}};
  InputQueue::Request request{};
  std::string_view stop_name;
  size_t delim_pos{data.find_first_of(',')}, cycles_counter{0};
  const size_t LongitudeIteration{1};
  double real_dist;
  for (; delim_pos != data.npos; data = data.substr(delim_pos + 2),
                                 delim_pos = data.find_first_of(','),
                                 ++cycles_counter) {
    switch (cycles_counter) {
    case 0:
      new_stop.pos.lat = std::stod(std::string{data.substr(0, delim_pos)});
      break;
    case 1:
      new_stop.pos.lng = std::stod(std::string{data.substr(0, delim_pos)});
      break;
    default:
      std::string_view link = data.substr(0, delim_pos);
      real_dist =
          std::stod(std::string{link.substr(0, link.find_first_of('m'))});
      stop_name = link.substr(data.find("to") + 3);
      new_stoplink.neighbours.emplace_back(stop_name, real_dist);
      break;
    }
  }

  if (cycles_counter == LongitudeIteration) {
    new_stop.pos.lng = std::stod(std::string{data.substr(0, delim_pos)});
  } else {
    real_dist = std::stod(std::string{data.substr(0, data.find_first_of('m'))});
    stop_name = data.substr(data.find("to") + 3);
    new_stoplink.neighbours.emplace_back(stop_name, real_dist);
  }

  request.stop_input.stop_ptr = &stops_input_queue_.emplace_back(new_stop);
  input_queue_.push_back({InputQueue::StopInsert, request});
  request.stoplink_input.stoplink_ptr =
      &stoplinks_input_queue_.emplace_back(new_stoplink);
  input_queue_.push_back({InputQueue::SLinkInsert, request});
}

void InputReader::InsertStop(const InputQueue &stop) {
  storage_.AddStop(*stop.data.stop_input.stop_ptr);
}

void InputReader::InsertStopLinks(const InputQueue &stop_links) {
  storage_.AddStopLinks(*stop_links.data.stoplink_input.stoplink_ptr);
}

void InputReader::ParseBusInfo(std::string_view object_name,
                               std::string_view data) {
  InputInfo::Bus new_bus{object_name, {}, {}};
  InputQueue::Request info{};
  auto delim_pos = data.find_first_of("->");
  new_bus.is_circular = data.at(delim_pos) == '>';
  for (; delim_pos != std::string_view::npos;
       data = data.substr(delim_pos + 2),
       delim_pos = data.find_first_of("->\n")) {
    new_bus.stops.push_back(data.substr(0, delim_pos - 1));
  }
  new_bus.stops.push_back(data);

  info.bus_input.bus_ptr = &buses_input_queue_.emplace_back(std::move(new_bus));
  input_queue_.push_back({InputQueue::BusInsert, info});
}

void InputReader::InsertBus(const InputQueue &bus) {
  storage_.AddBus(*bus.data.bus_input.bus_ptr);
}

void InputReader::Clear() {
  input_queue_.clear();
  stops_input_queue_.clear();
  stoplinks_input_queue_.clear();
  buses_input_queue_.clear();
}
