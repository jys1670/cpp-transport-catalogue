#include "serialization.h"

namespace serialization {

void Serializer::SerializeStops(const std::deque<core::data::Stop> &stops) {
  size_t counter{0};
  for (const auto &stop : stops) {
    Coordinates sr_coords;
    sr_coords.set_lat(stop.pos.lat);
    sr_coords.set_lng(stop.pos.lng);

    Stop sr_stop;
    sr_stop.set_name(stop.name);
    *sr_stop.mutable_coordinates() = std::move(sr_coords);
    stop_to_index_[stop.name] = counter++;

    *sr_catalogue_.add_stops() = std::move(sr_stop);
  }
}

void Serializer::SerializeBuses(const std::deque<core::data::Bus> &buses) {
  size_t counter{0};
  for (const auto &bus : buses) {
    Bus sr_bus;
    sr_bus.set_name(bus.name);
    sr_bus.set_is_circular(bus.is_circular);
    for (auto &stop_ptr : bus.stops) {
      sr_bus.add_stop_indexes(stop_to_index_.at(stop_ptr->name));
    }
    bus_to_index_[bus.name] = counter++;

    *sr_catalogue_.add_buses() = std::move(sr_bus);
  }
}

void Serializer::SerializeStopStats(const core::data::StopStorage &stop_stats) {
  for (const auto &[key, val] : stop_stats) {
    StopStats sr_stop_stats;
    sr_stop_stats.set_stop_index(stop_to_index_.at(key));
    for (const auto &bus : val.linked_buses) {
      sr_stop_stats.add_linked_buses_indexes(bus_to_index_.at(bus->name));
    }
    for (const auto &stop : val.linked_stops) {
      sr_stop_stats.add_linked_stops_indexes(
          stop_to_index_.at(stop.first->name));
      sr_stop_stats.add_linked_stops_distances(stop.second);
    }
    *sr_catalogue_.add_stopname_to_stop_stats() = std::move(sr_stop_stats);
  }
}

void Serializer::SerializeBusStats(const core::data::BusStorage &bus_stats) {
  for (const auto &[key, val] : bus_stats) {
    BusStats sr_bus_stats;
    sr_bus_stats.set_bus_index(bus_to_index_.at(key));
    sr_bus_stats.set_direct_length(val.direct_lenght);
    sr_bus_stats.set_real_length(val.real_length);
    sr_bus_stats.set_total_stops(val.total_stops);
    for (const auto &stop : val.unique_stops) {
      sr_bus_stats.add_uniq_stops_indexes(stop_to_index_.at(stop->name));
    }

    *sr_catalogue_.add_busname_to_bus_stats() = std::move(sr_bus_stats);
  }
}

void Serializer::SerializeRenderSettings(
    const graphics::RenderSettings &settings) {
  RenderSettings sr_settings;
  sr_settings.set_width(settings.width);
  sr_settings.set_height(settings.height);
  sr_settings.set_padding(settings.padding);
  sr_settings.set_line_width(settings.line_width);
  sr_settings.set_stop_radius(settings.stop_radius);
  sr_settings.set_bus_label_font_size(settings.bus_label_font_size);
  sr_settings.set_stop_label_font_size(settings.stop_label_font_size);
  sr_settings.set_underlayer_width(settings.underlayer_width);

  sr_settings.mutable_bus_label_offset()->set_x(settings.bus_label_offset.x);
  sr_settings.mutable_bus_label_offset()->set_y(settings.bus_label_offset.y);
  sr_settings.mutable_stop_label_offset()->set_x(settings.stop_label_offset.x);
  sr_settings.mutable_stop_label_offset()->set_y(settings.stop_label_offset.y);

  *(sr_settings.mutable_underlayer_color()) =
      SerializeColor(settings.underlayer_color);

  for (const graphics::svg::Color &color : settings.color_palette) {
    sr_settings.mutable_color_palette()->Add(SerializeColor(color));
  }

  *(sr_catalogue_.mutable_render_settings()) = sr_settings;
}

void Serializer::SerializeVertexIds(
    const std::vector<core::Vertex> &id_to_vertex) {
  for (const auto &vertex : id_to_vertex) {
    Vertex sr_vertex;
    sr_vertex.set_stop_name(vertex.GetStop()->name);
    sr_vertex.set_must_wait(vertex.GetWaitStatus());
    *(sr_catalogue_.add_id_to_vertex()) = sr_vertex;
  }
}

void Serializer::SerializeGraphRouterInternals(
    const graph::Router<double>::RoutesInternalData &routes_data) {
  Router &sr_router = *sr_catalogue_.mutable_router();
  for (const auto &data_list : routes_data) {
    RouteInternalDataList sr_data_list;
    for (const auto &data : data_list) {
      RouteInternalData sr_data;
      if (data) {
        sr_data.set_weight(data->weight);
      } else {
        sr_data.set_weight(-1);
        *(sr_data_list.add_route_data()) = sr_data;
        continue;
      }
      if (data->prev_edge) {
        sr_data.set_prev_edge(*(data->prev_edge));
        // sr_data.set_has_prev_edge(true);
      } else {
        sr_data.clear_prev_edge();
        // sr_data.set_has_prev_edge(false);
      }
      *(sr_data_list.add_route_data()) = std::move(sr_data);
    }
    *(sr_router.add_routes_data_list()) = std::move(sr_data_list);
  }
}

void Serializer::SerializeGraph(
    const graph::DirectedWeightedGraph<double> &graph) {
  Graph &sr_graph = *sr_catalogue_.mutable_router()->mutable_graph();
  for (size_t i = 0; i < graph.GetEdgeCount(); ++i) {
    *(sr_graph.add_edge()) = SerializeEdge(graph.GetEdge(i));
  }

  for (size_t i = 0; i < graph.GetVertexCount(); ++i) {
    *(sr_graph.add_incidence_lists()) =
        SerializeIncidenceLists(graph.GetIncidentEdges(i));
  }
}

void Serializer::SerializeToOstream(std::ostream *out) const {
  sr_catalogue_.SerializeToOstream(out);
}

serialization::Color
Serializer::SerializeColor(const graphics::svg::Color &color) {
  Color sr_color;
  if (auto ptr_string = std::get_if<std::string>(&color)) {
    sr_color.set_color_string(*ptr_string);
    return sr_color;
  }
  if (auto ptr_rgb = std::get_if<graphics::svg::Rgb>(&color)) {
    auto &sr_rgb = *sr_color.mutable_rgb();
    sr_rgb.set_red(ptr_rgb->red);
    sr_rgb.set_green(ptr_rgb->green);
    sr_rgb.set_blue(ptr_rgb->blue);
    return sr_color;
  }
  auto ptr_rgba = std::get_if<graphics::svg::Rgba>(&color);
  auto &sr_rgba = *sr_color.mutable_rgba();
  sr_rgba.set_red(ptr_rgba->red);
  sr_rgba.set_green(ptr_rgba->green);
  sr_rgba.set_blue(ptr_rgba->blue);
  sr_rgba.set_opacity(ptr_rgba->opacity);

  return sr_color;
}

Edge Serializer::SerializeEdge(const graph::Edge<double> &edge) {
  Edge sr_edge;
  sr_edge.set_from(edge.from);
  sr_edge.set_to(edge.to);
  sr_edge.set_weight(edge.weight);
  sr_edge.set_stop_count(edge.stop_count);
  sr_edge.set_bus_name(edge.bus->name);
  return sr_edge;
}

IncidenceList Serializer::SerializeIncidenceLists(
    graph::DirectedWeightedGraph<double>::IncidentEdgesRange range) {
  IncidenceList sr_list;
  for (auto it = range.begin(); it != range.end(); ++it) {
    sr_list.add_edge_id(*it);
  }
  return sr_list;
}

} // namespace serialization
