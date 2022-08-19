#include "map_renderer.h"
namespace graphics {

bool IsZero(double value) { return std::abs(value) < EPSILON; }

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
  return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
          (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

void MapRenderer::ImportRenderSettings(
    const serialization::TrCatalogue &sr_catalogue) {
  auto &settings = sr_catalogue.render_settings();
  settings_.width = settings.width();
  settings_.height = settings.height();
  settings_.padding = settings.padding();
  settings_.line_width = settings.line_width();
  settings_.stop_radius = settings.stop_radius();
  settings_.bus_label_font_size = settings.bus_label_font_size();
  settings_.stop_label_font_size = settings.stop_label_font_size();
  settings_.underlayer_width = settings.underlayer_width();

  settings_.bus_label_offset = {settings.bus_label_offset().x(),
                                settings.bus_label_offset().y()};
  settings_.stop_label_offset = {settings.stop_label_offset().x(),
                                 settings.stop_label_offset().y()};

  settings_.underlayer_color = DeserializeColor(settings.underlayer_color());

  settings_.color_palette.clear();
  for (int i = 0; i < settings.color_palette_size(); ++i) {
    settings_.color_palette.push_back(
        DeserializeColor(settings.color_palette(i)));
  }
}

void MapRenderer::ExportRenderSettings(serialization::Serializer &sr) {
  sr.SerializeRenderSettings(settings_);
}

svg::Color MapRenderer::DeserializeColor(const serialization::Color &color) {
  if (color.has_color_string()) {
    return color.color_string();
  }
  if (color.has_rgb()) {
    auto &col = color.rgb();
    return svg::Rgb(col.red(), col.green(), col.blue());
  }
  auto &col = color.rgba();
  return svg::Rgba(col.red(), col.green(), col.blue(), col.opacity());
}

std::string MapRenderer::RenderMap(core::data::RoutesData data) {

  SphereProjector projector{
      core::data::StopCoordsIterator{data.routes_stops.cbegin()},
      core::data::StopCoordsIterator{data.routes_stops.cend()}, settings_.width,
      settings_.height, settings_.padding};
  svg::Document doc;
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

void MapRenderer::LoadSettings(const io::json::Node &node) {
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

svg::Color MapRenderer::ParseColor(const io::json::Node &node) {
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

void MapRenderer::DrawRoutes(core::data::RoutesData &data,
                             SphereProjector &projector, svg::Document &doc) {
  size_t counter{0}, palsize{settings_.color_palette.size()};
  for (const auto bus : data.bus_stats) {
    if (!bus->unique_stops.empty()) {
      bool is_roundtrip = bus->bus_ptr->is_circular;

      svg::Polyline route;
      route.SetStrokeColor(settings_.color_palette.at(counter % palsize))
          .SetFillColor(svg::NoneColor)
          .SetStrokeWidth(settings_.line_width)
          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
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

void MapRenderer::DrawRouteNames(core::data::RoutesData &data,
                                 SphereProjector &projector,
                                 svg::Document &doc) {
  svg::Text name, substrate;
  substrate.SetOffset(settings_.bus_label_offset)
      .SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size))
      .SetFontFamily("Verdana")
      .SetFontWeight("bold")
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
  name.SetOffset(settings_.bus_label_offset)
      .SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size))
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

void MapRenderer::DrawStopSymbols(core::data::RoutesData &data,
                                  SphereProjector &projector,
                                  svg::Document &doc) {
  svg::Circle circ;
  circ.SetRadius(settings_.stop_radius).SetFillColor("white");
  for (const auto stop : data.routes_stops) {
    doc.Add(circ.SetCenter(projector(stop->pos)));
  }
}

void MapRenderer::DrawStopNames(core::data::RoutesData &data,
                                SphereProjector &projector,
                                svg::Document &doc) {
  svg::Text name, substrate;
  substrate.SetOffset(settings_.stop_label_offset)
      .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size))
      .SetFontFamily("Verdana")
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
  name.SetOffset(settings_.stop_label_offset)
      .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size))
      .SetFontFamily("Verdana")
      .SetFillColor("black");
  for (const auto stop : data.routes_stops) {
    substrate.SetData(stop->name).SetPosition(projector(stop->pos));
    name.SetData(stop->name).SetPosition(projector(stop->pos));
    doc.Add(substrate);
    doc.Add(name);
  }
}
} // namespace graphics
