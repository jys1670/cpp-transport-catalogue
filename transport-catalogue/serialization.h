#pragma once

#include "domain.h"
#include "graph.h"
#include "json.h"
#include "router.h"

#include <deque>
#include <functional>
#include <iostream>
#include <transport_catalogue.pb.h>

namespace serialization {

class Serializer {
public:
  void SerializeStops(const std::deque<data::Stop> &stops);
  void SerializeBuses(const std::deque<data::Bus> &buses);
  void SerializeStopStats(const data::StopStorage &stop_stats);
  void SerializeBusStats(const data::BusStorage &bus_stats);

  void SerializeRenderSettings(const input_info::RenderSettings &settings);

  void SerializeVertexIds(const std::vector<data::Vertex> &id_to_vertex);
  void SerializeGraphRouterInternals(
      const graph::Router<double>::RoutesInternalData &routes_data);
  void SerializeGraph(const graph::DirectedWeightedGraph<double> &graph);

  void SerializeToOstream(std::ostream *out) const;

private:
  TrCatalogue sr_catalogue_;
  std::unordered_map<std::string_view, size_t> stop_to_index_;
  std::unordered_map<std::string_view, size_t> bus_to_index_;

  static Color SerializeColor(const graphics::svg::Color &color);
  static Edge SerializeEdge(const graph::Edge<double> &edge);
  static IncidenceList SerializeIncidenceLists(
      graph::DirectedWeightedGraph<double>::IncidentEdgesRange range);
};

} // namespace serialization
