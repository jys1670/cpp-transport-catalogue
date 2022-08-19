#include "domain.h"

const geo::Coordinates &core::data::StopCoordsIterator::operator*() const {
  return (*wrapped_)->pos;
}

core::data::StopCoordsIterator::pointer
core::data::StopCoordsIterator::operator->() const {
  return &(*wrapped_)->pos;
}

bool core::data::StopCoordsIterator::operator==(
    const data::StopCoordsIterator &other) const {
  return wrapped_ == other.wrapped_;
}

bool core::data::StopCoordsIterator::operator!=(
    const data::StopCoordsIterator &other) const {
  return !(*this == other);
}

core::data::StopCoordsIterator core::data::StopCoordsIterator::operator++(int) {
  auto tmp = *this;
  ++wrapped_;
  return tmp;
}

core::data::StopCoordsIterator &core::data::StopCoordsIterator::operator++() {
  ++wrapped_;
  return *this;
}
size_t core::VertexHasher::operator()(const core::Vertex &obj) const {
  return (reinterpret_cast<size_t>(obj.GetStop()) << obj.GetWaitStatus());
}
core::Vertex &core::Vertex::SetStop(const core::data::Stop *st) {
  stop = st;
  return *this;
}
core::Vertex &core::Vertex::SetWait(bool wait) {
  must_wait = wait;
  return *this;
}
const core::data::Stop *core::Vertex::GetStop() const { return stop; }
bool core::Vertex::GetWaitStatus() const { return must_wait; }
bool core::Vertex::operator==(const core::Vertex &other) const {
  return stop == other.stop && must_wait == other.must_wait;
}
core::RouteAnswer::Wait &
core::RouteAnswer::Wait::SetStop(const core::data::Stop *ptr) {
  stop = ptr;
  return *this;
}
core::RouteAnswer::Wait &core::RouteAnswer::Wait::SetTime(double number) {
  time = number;
  return *this;
}
core::RouteAnswer::Bus &
core::RouteAnswer::Bus::SetBus(const core::data::Bus *ptr) {
  bus = ptr;
  return *this;
}
core::RouteAnswer::Bus &core::RouteAnswer::Bus::SetSpanCount(size_t number) {
  span_count = number;
  return *this;
}
core::RouteAnswer::Bus &core::RouteAnswer::Bus::SetTime(double number) {
  time = number;
  return *this;
}
core::data::BusStats &core::data::BusStats::SetBus(core::data::Bus *ptr) {
  bus_ptr = ptr;
  return *this;
}
core::data::BusStats &core::data::BusStats::SetTotalStops(size_t number) {
  total_stops = number;
  return *this;
}
core::data::BusStats &
core::data::BusStats::SetUniqueStops(std::unordered_set<Stop *> &&stops) {
  unique_stops = std::move(stops);
  return *this;
}
core::data::BusStats &core::data::BusStats::SetDirectLength(double val) {
  direct_lenght = val;
  return *this;
}
core::data::BusStats &core::data::BusStats::SetRealLength(double val) {
  real_length = val;
  return *this;
}
core::data::StopStats &core::data::StopStats::SetStop(core::data::Stop *ptr) {
  stop_ptr = ptr;
  return *this;
}
core::data::StopStats &
core::data::StopStats::AddLinkedBus(core::data::Bus *ptr) {
  linked_buses.insert(ptr);
  return *this;
}
core::data::StopStats &
core::data::StopStats::SetLinkedStopDistance(core::data::Stop *ptr,
                                             double distance) {
  linked_stops[ptr] = distance;
  return *this;
}
core::data::Bus &core::data::Bus::SetBusName(std::string_view str) {
  name = str;
  return *this;
}
core::data::Bus &core::data::Bus::SetCircular(bool value) {
  is_circular = value;
  return *this;
}
core::data::Bus &core::data::Bus::AddStop(core::data::Stop *ptr) {
  stops.emplace_back(ptr);
  return *this;
}
core::data::Stop &core::data::Stop::SetStopName(std::string_view str) {
  name = str;
  return *this;
}
core::data::Stop &core::data::Stop::SetCoordinates(geo::Coordinates value) {
  pos = value;
  return *this;
}
