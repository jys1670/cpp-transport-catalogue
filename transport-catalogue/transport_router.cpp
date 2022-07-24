#include <algorithm>

#include "transport_router.h"

TransportRouter::TransportRouter(const TransportCatalogue &catalogue)
    : catalogue_{catalogue} {}

std::optional<RouteAnswer>
TransportRouter::FindFastestRoute(std::string_view from, std::string_view to) {
  if (!graph_finished_) {
    GenerateGraph();
    router_ = std::make_unique<graph::Router<double>>(graph_);
    graph_finished_ = true;
  }

  Vertex vfrom = Vertex()
                     .SetStop(catalogue_.GetStopInfo(from)->stop_ptr)
                     .SetWait(true),
         vto = Vertex()
                   .SetStop(catalogue_.GetStopInfo(to)->stop_ptr)
                   .SetWait(true);
  auto fastest_path =
      router_->BuildRoute(vertex_to_id_.at(vfrom), vertex_to_id_.at(vto));
  if (fastest_path) {
    return GenerateAnswer(fastest_path.value());
  }
  return std::nullopt;
}

void TransportRouter::GenerateGraph() {

  std::vector<const DataStorage::Stop *> stops = catalogue_.GetAllStops();
  std::vector<const DataStorage::Bus *> buses;
  for (const auto &[_, stats] : catalogue_.GetBusStatsMap()) {
    buses.push_back(stats.bus_ptr);
  }

  graph_ = graph::DirectedWeightedGraph<double>(2 * stops.size());
  GenerateVertexes(stops);
  for (auto bus : buses) {
    InsertAllEdgesIntoGraph(bus);
  }
}

void TransportRouter::GenerateVertexes(
    std::vector<const DataStorage::Stop *> &stops) {

  for (auto stop : stops) {
    auto wait = Vertex().SetStop(stop).SetWait(true);
    auto normal = Vertex().SetStop(stop).SetWait(false);
    vertex_to_id_[wait] = id_to_vertex_.size();
    id_to_vertex_.push_back(wait);
    vertex_to_id_[normal] = id_to_vertex_.size();
    id_to_vertex_.push_back(normal);
  }
}

void TransportRouter::InsertAllEdgesIntoGraph(const DataStorage::Bus *bus) {

  for (auto stop : bus->stops) {
    auto wait_id = vertex_to_id_.at(Vertex().SetStop(stop).SetWait(true));
    auto norm_id = vertex_to_id_.at(Vertex().SetStop(stop).SetWait(false));
    graph_.AddEdge(Edge()
                       .SetFromVertex(wait_id)
                       .SetToVertex(norm_id)
                       .SetWeight(settings_.bus_wait_time)
                       .SetBus(bus)
                       .SetStopCount(0));
  }
  InsertEdgesBetweenStops(bus->stops.begin(), bus->stops.end(), bus);
  if (bus->is_circular) {
    return;
  } else {
    InsertEdgesBetweenStops(bus->stops.rbegin(), bus->stops.rend(), bus);
  }
}

RouteAnswer
TransportRouter::GenerateAnswer(const graph::Router<double>::RouteInfo &path) {

  RouteAnswer result;
  for (auto edge_id : path.edges) {
    auto curr_edge = graph_.GetEdge(edge_id);
    if (!curr_edge.stop_count) {
      result.items.emplace_back(RouteAnswer::Bus()
                                    .SetBus(curr_edge.bus)
                                    .SetSpanCount(curr_edge.stop_count)
                                    .SetTime(curr_edge.weight));
    } else {
      result.items.emplace_back(
          RouteAnswer::Wait()
              .SetStop(id_to_vertex_.at(curr_edge.from).GetStop())
              .SetTime(curr_edge.weight));
    }
  }
  result.total_time = path.weight;
  return result;
}

TransportRouter::Vertex &
TransportRouter::Vertex::SetStop(const DataStorage::Stop *st) {
  stop = st;
  return *this;
}

TransportRouter::Vertex &TransportRouter::Vertex::SetWait(bool wait) {
  must_wait = wait;
  return *this;
}

const DataStorage::Stop *TransportRouter::Vertex::GetStop() const {
  return stop;
}

bool TransportRouter::Vertex::GetWaitStatus() const { return must_wait; }

bool TransportRouter::Vertex::operator==(
    const TransportRouter::Vertex &other) const {
  return stop == other.stop && must_wait == other.must_wait;
}
