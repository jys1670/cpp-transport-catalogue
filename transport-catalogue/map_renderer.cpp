#include "map_renderer.h"

bool IsZero(double value) { return std::abs(value) < EPSILON; }

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
  return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
          (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

std::string MapRenderer::RenderMap(DataStorage::RoutesData data) {
  using namespace DataStorage;
  using namespace svg;
  SphereProjector projector{StopCoordsIterator{data.routes_stops.cbegin()},
                            StopCoordsIterator{data.routes_stops.cend()},
                            settings_.width, settings_.height,
                            settings_.padding};
  Document doc;
  std::sort(data.bus_stats.begin(), data.bus_stats.end(),
            [](const auto *lhs, const auto *rhs) {
              return lhs->bus_ptr->name < rhs->bus_ptr->name;
            });
  std::sort(
      data.routes_stops.begin(), data.routes_stops.end(),
      [](const auto *lhs, const auto *rhs) { return lhs->name < rhs->name; });
  DrawRoutes(data, projector, doc);
  DrawRouteNames(data, projector, doc);
  DrawStopSymbols(data, projector, doc);
  DrawStopNames(data, projector, doc);

  std::ostringstream tmp{};
  doc.Render(tmp);
  return tmp.str();
}

void MapRenderer::LoadSettings(const json::Node &node) {
  const auto &render_map = node.AsMap();
  settings_.width = render_map.at("width").AsDouble();
  settings_.height = render_map.at("height").AsDouble();
  settings_.padding = render_map.at("padding").AsDouble();
  settings_.line_width = render_map.at("line_width").AsDouble();
  settings_.stop_radius = render_map.at("stop_radius").AsDouble();
  settings_.bus_label_font_size = render_map.at("bus_label_font_size").AsInt();
  const auto &b_offs = render_map.at("bus_label_offset").AsArray();
  settings_.bus_label_offset = {b_offs.at(0).AsDouble(),
                                b_offs.at(1).AsDouble()};
  settings_.stop_label_font_size =
      render_map.at("stop_label_font_size").AsInt();
  const auto &s_offs = render_map.at("stop_label_offset").AsArray();
  settings_.stop_label_offset = {s_offs.at(0).AsDouble(),
                                 s_offs.at(1).AsDouble()};
  settings_.underlayer_color = ParseColor(render_map.at("underlayer_color"));
  settings_.underlayer_width = render_map.at("underlayer_width").AsDouble();
  settings_.color_palette.clear();
  for (const auto &color : render_map.at("color_palette").AsArray()) {
    settings_.color_palette.push_back(ParseColor(color));
  }
}

svg::Color MapRenderer::ParseColor(const json::Node &node) {
  if (node.IsString())
    return {node.AsString()};
  const auto &carr = node.AsArray();
  if (carr.size() == 3) {
    return svg::Rgb{static_cast<uint8_t>(carr.at(0).AsInt()),
                    static_cast<uint8_t>(carr.at(1).AsInt()),
                    static_cast<uint8_t>(carr.at(2).AsInt())};
  }
  return svg::Rgba{static_cast<uint8_t>(carr.at(0).AsInt()),
                   static_cast<uint8_t>(carr.at(1).AsInt()),
                   static_cast<uint8_t>(carr.at(2).AsInt()),
                   carr.at(3).AsDouble()};
}

void MapRenderer::DrawRoutes(DataStorage::RoutesData &data,
                             SphereProjector &projector, svg::Document &doc) {
  using namespace DataStorage;
  using namespace svg;
  size_t counter{0}, palsize{settings_.color_palette.size()};
  for (const auto bus : data.bus_stats) {
    if (!bus->unique_stops.empty()) {
      bool is_roundtrip = bus->bus_ptr->is_circular;

      svg::Polyline route;
      route.SetStrokeColor(settings_.color_palette.at(counter % palsize))
          .SetFillColor(NoneColor)
          .SetStrokeWidth(settings_.line_width)
          .SetStrokeLineCap(StrokeLineCap::ROUND)
          .SetStrokeLineJoin(StrokeLineJoin::ROUND);
      for (auto stop : bus->bus_ptr->stops) {
        route.AddPoint(projector(stop->pos));
      }
      if (!is_roundtrip) {
        for (auto it = next(bus->bus_ptr->stops.rbegin()),
                  end = bus->bus_ptr->stops.rend();
             it != end; ++it) {
          route.AddPoint(projector((*it)->pos));
        }
      }
      ++counter;
      doc.Add(std::move(route));
      ;
    }
  }
}

void MapRenderer::DrawRouteNames(DataStorage::RoutesData &data,
                                 SphereProjector &projector,
                                 svg::Document &doc) {
  using namespace DataStorage;
  using namespace svg;
  svg::Text name, substrate;
  substrate.SetOffset(settings_.bus_label_offset)
      .SetFontSize(settings_.bus_label_font_size)
      .SetFontFamily("Verdana")
      .SetFontWeight("bold")
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineJoin(StrokeLineJoin::ROUND)
      .SetStrokeLineCap(StrokeLineCap::ROUND);
  name.SetOffset(settings_.bus_label_offset)
      .SetFontSize(settings_.bus_label_font_size)
      .SetFontFamily("Verdana")
      .SetFontWeight("bold");
  size_t counter{0}, palsize{settings_.color_palette.size()};
  for (const auto bus : data.bus_stats) {
    if (!bus->unique_stops.empty()) {
      bool is_roundtrip = bus->bus_ptr->is_circular;
      const auto &stop1 = bus->bus_ptr->stops.front();
      substrate.SetData(bus->bus_ptr->name).SetPosition(projector(stop1->pos));
      name.SetData(bus->bus_ptr->name)
          .SetPosition(projector(stop1->pos))
          .SetFillColor(settings_.color_palette.at(counter % palsize));
      doc.Add(substrate);
      doc.Add(name);
      if (!is_roundtrip) {
        const auto &stop2 = bus->bus_ptr->stops.back();
        if (stop1 != stop2) {
          substrate.SetData(bus->bus_ptr->name)
              .SetPosition(projector(stop2->pos));
          name.SetData(bus->bus_ptr->name).SetPosition(projector(stop2->pos));
          doc.Add(substrate);
          doc.Add(name);
        }
      }
      ++counter;
    }
  }
}

void MapRenderer::DrawStopSymbols(DataStorage::RoutesData &data,
                                  SphereProjector &projector,
                                  svg::Document &doc) {
  using namespace DataStorage;
  using namespace svg;
  svg::Circle circ;
  circ.SetRadius(settings_.stop_radius).SetFillColor("white");
  for (const auto stop : data.routes_stops) {
    doc.Add(circ.SetCenter(projector(stop->pos)));
  }
}

void MapRenderer::DrawStopNames(DataStorage::RoutesData &data,
                                SphereProjector &projector,
                                svg::Document &doc) {
  using namespace DataStorage;
  using namespace svg;
  svg::Text name, substrate;
  substrate.SetOffset(settings_.stop_label_offset)
      .SetFontSize(settings_.stop_label_font_size)
      .SetFontFamily("Verdana")
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineJoin(StrokeLineJoin::ROUND)
      .SetStrokeLineCap(StrokeLineCap::ROUND);
  name.SetOffset(settings_.stop_label_offset)
      .SetFontSize(settings_.stop_label_font_size)
      .SetFontFamily("Verdana")
      .SetFillColor("black");
  for (const auto stop : data.routes_stops) {
    substrate.SetData(stop->name).SetPosition(projector(stop->pos));
    name.SetData(stop->name).SetPosition(projector(stop->pos));
    doc.Add(substrate);
    doc.Add(name);
  }
}
