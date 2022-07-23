/*!
 * \file transport_router.h
 * \brief Converts TransportCatalogue data into directed graph and vice versa
 */

#pragma once

#include "domain.h"
#include "json.h"
#include "router.h"
#include "transport_catalogue.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

//! Format independent description of the fastest path
struct RouteAnswer {
  struct Wait {
    const DataStorage::Stop *stop;
    double time;
  };
  struct Bus {
    const DataStorage::Bus *bus;
    int span_count;
    double time;
  };
  using Item = std::variant<Wait, Bus>;
  double total_time;
  std::vector<Item> items;
};

//!
class TransportRouter {
public:
  /*!
   * Constructor for the class
   * \param[in] catalogue database whose content will be used to generate
   * directed graph
   */
  TransportRouter(const TransportCatalogue &catalogue);

  /*!
   * Updates routing parameters such as velocity, wait time, etc
   * \param[in] node json::Dict that defines all parameters
   */
  void LoadSettings(const json::Node &node) {
    const auto &settings = node.AsMap();
    bus_wait_time_ = settings.at("bus_wait_time").AsDouble();
    bus_velocity_ = settings.at("bus_velocity").AsDouble() * 1000.0 / 60.0;
  }

  /*!
   * Find the fastest (in terms of time) path between two existing (!) stops
   * \param[in] from Starting stop name
   * \param[in] to Destination stop name
   * \return Path description
   */
  std::optional<RouteAnswer> FindFastestRoute(std::string_view from,
                                              std::string_view to);

private:
  /*!
   * Represents real world entity DataStorage::Stop (and its properties) as
   * graph vertex. Combination of all class fields defines unique graph vertex.
   */
  class Vertex {
  public:
    Vertex &SetStop(const DataStorage::Stop *st);
    Vertex &SetWait(bool wait);
    const DataStorage::Stop *GetStop() const;
    bool GetWaitStatus() const;
    bool operator==(const Vertex &other) const;

  private:
    const DataStorage::Stop *stop{nullptr};
    bool must_wait{false};
  };

  struct VertexHasher {
    size_t operator()(const Vertex &obj) const {
      return (reinterpret_cast<size_t>(obj.GetStop()) << obj.GetWaitStatus());
    }
  };

  const TransportCatalogue &catalogue_;
  double bus_wait_time_{};
  double bus_velocity_{};

  graph::DirectedWeightedGraph<double> graph_{};
  bool graph_finished_{false};

  // router needs finished graph as constructor, thus pointer is used
  std::unique_ptr<graph::Router<double>> router_{};

  std::unordered_map<Vertex, size_t, VertexHasher> vertex_to_id_{};
  std::vector<Vertex> id_to_vertex_{};

  void GenerateGraph();

  void GenerateVertexes(std::vector<const DataStorage::Stop *> &stops);

  void InsertAllEdgesIntoGraph(const DataStorage::Bus *bus);

  template <typename InputIt>
  void InsertEdgesBetweenStops(InputIt begin, InputIt end,
                               const DataStorage::Bus *bus);

  RouteAnswer GenerateAnswer(const graph::Router<double>::RouteInfo &path);
};

template <typename InputIt>
void TransportRouter::InsertEdgesBetweenStops(InputIt begin, InputIt end,
                                              const DataStorage::Bus *bus) {
  for (auto it1 = begin; it1 != end; ++it1) {
    double tot_dist{0};
    for (auto it2 = next(it1); it2 != end; ++it2) {
      auto norm_id = vertex_to_id_.at(Vertex().SetStop(*it1).SetWait(false));
      auto wait_id = vertex_to_id_.at(Vertex().SetStop(*it2).SetWait(true));
      auto stops_between = static_cast<size_t>(std::distance(it1, it2));
      auto curr_dist =
          catalogue_.GetStopsRealDist((*prev(it2))->name, (*it2)->name).value();
      tot_dist += curr_dist;
      graph_.AddEdge(
          {norm_id, wait_id, tot_dist / bus_velocity_, bus, stops_between});
    }
  }
}