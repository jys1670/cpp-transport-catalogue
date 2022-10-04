#include "domain.h"

const geo::Coordinates &data::StopCoordsIterator::operator*() const {
  return (*wrapped_)->pos;
}

data::StopCoordsIterator::pointer data::StopCoordsIterator::operator->() const {
  return &(*wrapped_)->pos;
}

bool data::StopCoordsIterator::operator==(
    const StopCoordsIterator &other) const {
  return wrapped_ == other.wrapped_;
}

bool data::StopCoordsIterator::operator!=(
    const StopCoordsIterator &other) const {
  return !(*this == other);
}

data::StopCoordsIterator data::StopCoordsIterator::operator++(int) {
  auto tmp = *this;
  ++wrapped_;
  return tmp;
}

data::StopCoordsIterator &data::StopCoordsIterator::operator++() {
  ++wrapped_;
  return *this;
}
size_t data::VertexHasher::operator()(const data::Vertex &obj) const {
  return (reinterpret_cast<size_t>(obj.GetStop()) << obj.GetWaitStatus());
}
data::Vertex &data::Vertex::SetStop(const Stop *st) {
  stop = st;
  return *this;
}
data::Vertex &data::Vertex::SetWait(bool wait) {
  must_wait = wait;
  return *this;
}
const data::Stop *data::Vertex::GetStop() const { return stop; }
bool data::Vertex::GetWaitStatus() const { return must_wait; }
bool data::Vertex::operator==(const data::Vertex &other) const {
  return stop == other.stop && must_wait == other.must_wait;
}
data::RouteAnswer::Wait &data::RouteAnswer::Wait::SetStop(const Stop *ptr) {
  stop = ptr;
  return *this;
}
data::RouteAnswer::Wait &data::RouteAnswer::Wait::SetTime(double number) {
  time = number;
  return *this;
}
data::RouteAnswer::Bus &data::RouteAnswer::Bus::SetBus(const data::Bus *ptr) {
  bus = ptr;
  return *this;
}
data::RouteAnswer::Bus &data::RouteAnswer::Bus::SetSpanCount(size_t number) {
  span_count = number;
  return *this;
}
data::RouteAnswer::Bus &data::RouteAnswer::Bus::SetTime(double number) {
  time = number;
  return *this;
}
data::BusStats &data::BusStats::SetBus(Bus *ptr) {
  bus_ptr = ptr;
  return *this;
}
data::BusStats &data::BusStats::SetTotalStops(size_t number) {
  total_stops = number;
  return *this;
}
data::BusStats &
data::BusStats::SetUniqueStops(std::unordered_set<data::Stop *> &&stops) {
  unique_stops = std::move(stops);
  return *this;
}
data::BusStats &data::BusStats::SetDirectLength(double val) {
  direct_lenght = val;
  return *this;
}
data::BusStats &data::BusStats::SetRealLength(double val) {
  real_length = val;
  return *this;
}
data::StopStats &data::StopStats::SetStop(Stop *ptr) {
  stop_ptr = ptr;
  return *this;
}
data::StopStats &data::StopStats::AddLinkedBus(Bus *ptr) {
  linked_buses.insert(ptr);
  return *this;
}
data::StopStats &data::StopStats::SetLinkedStopDistance(Stop *ptr,
                                                        double distance) {
  linked_stops[ptr] = distance;
  return *this;
}
data::Bus &data::Bus::SetBusName(std::string_view str) {
  name = str;
  return *this;
}
data::Bus &data::Bus::SetCircular(bool value) {
  is_circular = value;
  return *this;
}
data::Bus &data::Bus::AddStop(Stop *ptr) {
  stops.emplace_back(ptr);
  return *this;
}
data::Stop &data::Stop::SetStopName(std::string_view str) {
  name = str;
  return *this;
}
data::Stop &data::Stop::SetCoordinates(geo::Coordinates value) {
  pos = value;
  return *this;
}
