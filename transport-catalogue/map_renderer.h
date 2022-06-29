#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "json.h"
#include "svg.h"

inline const double EPSILON = 1e-6;
bool IsZero(double value);

//! Functor for mapping physical to screen coordinates
class SphereProjector {
public:
  /*!
  Constructor for the class
  \param[in] points_begin iterator pointing to geo::Coordinates container begin
  \param[in] points_end iterator pointing to geo::Coordinates container end
  \param[in] max_width pixel width of "project to" area
  \param[in] max_height pixel height of "project to" area
  \param[in] padding "project to" area padding size in pixels
  */
  template <typename PointInputIt>
  SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                  double max_width, double max_height, double padding);

  svg::Point operator()(geo::Coordinates coords) const;

private:
  double padding_;
  double min_lon_ = 0;
  double max_lat_ = 0;
  double zoom_coeff_ = 0;
};

class MapRenderer {
public:
  std::string RenderMap(DataStorage::RoutesData data);
  void LoadSettings(const json::Node &node);

private:
  struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
  } settings_;

  static svg::Color ParseColor(const json::Node &node);

  void DrawRoutes(DataStorage::RoutesData &data, SphereProjector &projector,
                  svg::Document &doc);

  void DrawRouteNames(DataStorage::RoutesData &data, SphereProjector &projector,
                      svg::Document &doc);

  void DrawStopSymbols(DataStorage::RoutesData &data,
                       SphereProjector &projector, svg::Document &doc);

  void DrawStopNames(DataStorage::RoutesData &data, SphereProjector &projector,
                     svg::Document &doc);
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin,
                                 PointInputIt points_end, double max_width,
                                 double max_height, double padding)
    : padding_(padding) //
{

  if (points_begin == points_end) {
    return;
  }

  const auto [left_it, right_it] =
      std::minmax_element(points_begin, points_end,
                          [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
  min_lon_ = left_it->lng;
  const double max_lon = right_it->lng;

  const auto [bottom_it, top_it] =
      std::minmax_element(points_begin, points_end,
                          [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
  const double min_lat = bottom_it->lat;
  max_lat_ = top_it->lat;

  std::optional<double> width_zoom;
  if (!IsZero(max_lon - min_lon_)) {
    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
  }

  std::optional<double> height_zoom;
  if (!IsZero(max_lat_ - min_lat)) {
    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
  }

  if (width_zoom && height_zoom) {
    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
  } else if (width_zoom) {
    zoom_coeff_ = *width_zoom;
  } else if (height_zoom) {
    zoom_coeff_ = *height_zoom;
  }
}