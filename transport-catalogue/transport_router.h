/*!
 * \file transport_router.h
 * \brief Converts TransportCatalogue data into directed graph and vice versa
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "domain.h"
#include "json.h"
#include "router.h"
#include "serialization.h"
#include "transport_catalogue.h"

namespace core {

class TransportRouter {
public:
  using Edge = graph::Edge<double>;
  /*!
   * Constructor for the class
   * \param[in] catalogue database whose content will be used to generate
   * directed graph
   */
  TransportRouter(const TransportCatalogue &catalogue);

  void ImportState(const serialization::TrCatalogue &sr_catalogue);
  void ExportState(serialization::Serializer &sr);

  /*!
   * Updates routing parameters such as velocity, wait time, etc
   * \param[in] node json::Dict that defines all parameters
   */
  void LoadSettings(const json::Node &node) {
    const auto &settings = node.AsMap();
    settings_.bus_wait_time = settings.at("bus_wait_time").AsDouble();
    settings_.bus_velocity =
        settings.at("bus_velocity").AsDouble() * 1000.0 / 60.0;
  }

  /*!
   * Find the fastest (in terms of time) path between two existing (!) stops
   * \param[in] from Starting stop name
   * \param[in] to Destination stop name
   * \return Path description
   */
  std::optional<data::RouteAnswer> FindFastestRoute(std::string_view from,
                                              std::string_view to);

private:
  const TransportCatalogue &catalogue_;
  struct Settings {
    double bus_wait_time{};
    double bus_velocity{};
  } settings_;

  graph::DirectedWeightedGraph<double> graph_{};
  bool graph_finished_{false};

  // router needs finished graph as constructor, thus pointer is used
  std::unique_ptr<graph::Router<double>> router_{};

  std::unordered_map<data::Vertex, size_t, data::VertexHasher> vertex_to_id_{};
  std::vector<data::Vertex> id_to_vertex_{};

  void GenerateGraph();

  void GenerateVertexes(std::vector<const data::Stop *> &stops);

  void InsertAllEdgesIntoGraph(const data::Bus *bus);

  template <typename InputIt>
  void InsertEdgesBetweenStops(InputIt begin, InputIt end,
                               const data::Bus *bus);

  data::RouteAnswer GenerateAnswer(const graph::Router<double>::RouteInfo &path);

  void ImportGraph(const serialization::Graph &sr_graph);

  void ImportRouter(const serialization::Router &sr_router);

  void ImportVertexIds(const serialization::TrCatalogue &sr_catalogue);
};

template <typename InputIt>
void TransportRouter::InsertEdgesBetweenStops(InputIt begin, InputIt end,
                                              const data::Bus *bus) {
  for (auto it1 = begin; it1 != end; ++it1) {
    double tot_dist{0};
    for (auto it2 = next(it1); it2 != end; ++it2) {
      auto norm_id = vertex_to_id_.at(data::Vertex().SetStop(*it1).SetWait(false));
      auto wait_id = vertex_to_id_.at(data::Vertex().SetStop(*it2).SetWait(true));
      auto stops_between = static_cast<size_t>(std::distance(it1, it2));
      auto curr_dist =
          catalogue_.GetStopsRealDist((*prev(it2))->name, (*it2)->name).value();
      tot_dist += curr_dist;
      graph_.AddEdge(Edge()
                         .SetFromVertex(norm_id)
                         .SetToVertex(wait_id)
                         .SetWeight(tot_dist / settings_.bus_velocity)
                         .SetBus(bus)
                         .SetStopCount(stops_between));
    }
  }
}

} // namespace data