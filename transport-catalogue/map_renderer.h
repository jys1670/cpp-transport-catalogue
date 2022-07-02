/*!
 * \file map_renderer.h
 * \brief Objects used for visualization of stops and routes
 */
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

//! Functor used to map between physical and screen coordinates
class SphereProjector {
public:
  /*!
   * Constructor for the class
   * \param[in] points_begin iterator pointing to geo::Coordinates container
   * begin
   * \param[in] points_end iterator pointing to geo::Coordinates container
   * end
   * \param[in] max_width pixel width of "project to" area \param[in]
   * max_height pixel height of "project to" area \param[in] padding "project
   * to" area padding size in pixels
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
  /*!
   * Constructs map image
   * \param[in] data routes to be drawn and all their stops
   * \param[out] map Actual SVG stored as string
   */
  std::string RenderMap(DataStorage::RoutesData data);
  /*!
   * Updates image generation settings
   * \param[in] node json::Dict that defines all fields of RenderSettings
   */
  void LoadSettings(const json::Node &node);

private:
  //! MapRenderer settings storage
  struct RenderSettings {
    //! Map width (in pixels)
    double width{};
    //! Map height (in pixels)
    double height{};
    //! Map padding (in pixels)
    double padding{};
    /*!
     * Route lines <a
     * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/stroke-width">stroke-width</a>
     * attribute (in pixels)
     */
    double line_width{};
    //! Stops are drawn as white circles with this radius (in pixels)
    double stop_radius{};
    //! Route names font size (in pixels)
    int bus_label_font_size{};
    /*!
     * Route names position <a
     * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/dx">dx</a>
     * and <a
     * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/dy">dy</a>
     * (in pixels)
     */
    svg::Point bus_label_offset{};
    //! %Stop names font size (in pixels)
    int stop_label_font_size{};
    /*!
     * %Stop names position <a
     * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/dx">dx</a>
     * and <a
     * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/dy">dy</a>
     * (in pixels)
     */
    svg::Point stop_label_offset{};
    //! Stops and routes background font (stroke effect) color
    svg::Color underlayer_color{};
    /*!
     * Stops and routes background font (stroke effect) <a
     * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/stroke-width">stroke-width</a>
     */
    double underlayer_width{};
    //! Cyclically used colors for stops and routes
    std::vector<svg::Color> color_palette{};
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
    : padding_(padding) {

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