#include <algorithm>

#include "transport_router.h"
namespace core {
TransportRouter::TransportRouter(const core::TransportCatalogue &catalogue)
    : catalogue_{catalogue} {}

void TransportRouter::ExportState(serialization::Serializer &sr) {
  GenerateGraph();
  sr.SerializeVertexIds(id_to_vertex_);
  sr.SerializeGraphRouterInternals(router_->GetRoutesInternalData());
  sr.SerializeGraph(graph_);
}

void TransportRouter::ImportState(
    const serialization::TrCatalogue &sr_catalogue) {
  ImportGraph(sr_catalogue.router().graph());
  ImportRouter(sr_catalogue.router());
  ImportVertexIds(sr_catalogue);
}

std::optional<RouteAnswer>
TransportRouter::FindFastestRoute(std::string_view from, std::string_view to) {
  if (!graph_finished_) {
    GenerateGraph();
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
  if (!graph_finished_) {
    std::vector<const data::Stop *> stops = catalogue_.GetAllStops();
    std::vector<const data::Bus *> buses;
    for (const auto &[_, stats] : catalogue_.GetBusStatsMap()) {
      buses.push_back(stats.bus_ptr);
    }

    graph_ = graph::DirectedWeightedGraph<double>(2 * stops.size());
    GenerateVertexes(stops);
    for (auto bus : buses) {
      InsertAllEdgesIntoGraph(bus);
    }
    router_ = std::make_unique<graph::Router<double>>(graph_);
    graph_finished_ = true;
  }
}

void TransportRouter::GenerateVertexes(std::vector<const data::Stop *> &stops) {

  for (auto stop : stops) {
    auto wait = Vertex().SetStop(stop).SetWait(true);
    auto normal = Vertex().SetStop(stop).SetWait(false);
    vertex_to_id_[wait] = id_to_vertex_.size();
    id_to_vertex_.push_back(wait);
    vertex_to_id_[normal] = id_to_vertex_.size();
    id_to_vertex_.push_back(normal);
  }
}

void TransportRouter::InsertAllEdgesIntoGraph(const data::Bus *bus) {

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
    if (curr_edge.stop_count) {
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

void TransportRouter::ImportGraph(const serialization::Graph &sr_graph) {
  using IncList = graph::DirectedWeightedGraph<double>::IncidenceList;

  const auto &bus_stats = catalogue_.GetBusStatsMap();
  std::vector<Edge> edges;
  for (int i = 0; i < sr_graph.edge_size(); ++i) {
    edges.emplace_back(
        graph::Edge<double>{}
            .SetFromVertex(sr_graph.edge(i).from())
            .SetToVertex(sr_graph.edge(i).to())
            .SetWeight(sr_graph.edge(i).weight())
            .SetBus(bus_stats.at(sr_graph.edge(i).bus_name()).bus_ptr)
            .SetStopCount(sr_graph.edge(i).stop_count()));
  }
  std::vector<IncList> inc_lists;
  for (int i = 0; i < sr_graph.incidence_lists_size(); ++i) {
    IncList inc_list;
    for (int k = 0; k < sr_graph.incidence_lists(i).edge_id_size(); ++k) {
      inc_list.emplace_back(sr_graph.incidence_lists(i).edge_id(k));
    }
    inc_lists.emplace_back(inc_list);
  }
  graph_.SetEdges(std::move(edges)).SetIncidenceLists(std::move(inc_lists));
  graph_finished_ = true;
}

void TransportRouter::ImportRouter(const serialization::Router &sr_router) {
  using RoutesData = graph::Router<double>::RoutesInternalData;
  using Data = graph::Router<double>::RouteInternalData;

  RoutesData routes_data{};
  for (int i = 0; i < sr_router.routes_data_list_size(); ++i) {
    auto &sr_list = sr_router.routes_data_list(i);
    std::vector<std::optional<Data>> new_list;
    for (int j = 0; j < sr_list.route_data_size(); ++j) {
      auto &sr_data = sr_list.route_data(j);
      if (sr_data.weight() < 0) {
        new_list.emplace_back(std::nullopt);
        continue;
      }
      Data new_data{};
      new_data.weight = sr_data.weight();
      if (sr_data.has_prev_edge()) {
        new_data.prev_edge =
            std::make_optional<graph::EdgeId>(sr_data.prev_edge());
      } else {
        new_data.prev_edge = std::nullopt;
      }
      new_list.emplace_back(new_data);
    }
    routes_data.emplace_back(std::move(new_list));
  }
  router_ = std::make_unique<graph::Router<double>>(graph_);
  router_->SetRouterInternalData(std::move(routes_data));
}

void TransportRouter::ImportVertexIds(
    const serialization::TrCatalogue &sr_catalogue) {
  id_to_vertex_.clear();
  for (int i = 0; i < sr_catalogue.id_to_vertex_size(); ++i) {
    id_to_vertex_.emplace_back(
        Vertex{}
            .SetStop(catalogue_
                         .GetStopInfo(sr_catalogue.id_to_vertex(i).stop_name())
                         ->stop_ptr)
            .SetWait(sr_catalogue.id_to_vertex(i).must_wait()));
  }

  vertex_to_id_.clear();
  for (size_t i = 0; i < id_to_vertex_.size(); ++i) {
    vertex_to_id_[id_to_vertex_[i]] = i;
  }
}
} // namespace core