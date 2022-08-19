/*!
 * \file domain.h
 * \brief Header file with widely used independent types
 *
 * Contains declarations and definitions of all kinds of types,
 * that are used in composition with other parts of project: structures to
 * transfer information between different modules, iterators over these
 * structures elements and so on
 */
#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"
#include "svg.h"

//! Cheap to copy objects used for information transfer to TransportCatalogue
namespace io {

//! %Stop name and position (transfer)
struct Stop {
  std::string_view name{};
  geo::Coordinates pos{};
};

//! For a given Stop contains other linked (connected) stops (names and
//! distances)
struct StopLink {
  std::string_view stop_name; //!< "From" stop
  std::vector<std::pair<std::string_view, double>>
      neighbours; //!< Vector of "to" stops (names and distances)
};

//! Route name and stops (transfer)
struct Bus {
  std::string_view name;
  std::vector<std::string_view> stops;
  bool is_circular;
};

//! Dummy type used to specify program output format
enum class OutputFormat {
  Json,
  None,
};
} // namespace io

namespace core {

//! Objects solely used for information storage (mainly in TransportCatalogue)
namespace data {

//! %Stop name and position (storage)
struct Stop {
  Stop &SetStopName(std::string_view str);
  Stop &SetCoordinates(geo::Coordinates value);
  std::string name{};
  geo::Coordinates pos{};
};

//! Route name and stops (storage)
struct Bus {
  Bus &SetBusName(std::string_view str);
  Bus &SetCircular(bool value);
  Bus &AddStop(Stop *ptr);
  std::string name{};
  std::vector<Stop *> stops{};
  bool is_circular{};
};

//! Wrapper for Stop, used to painlessly add extra information about stop
struct StopStats {
  StopStats &SetStop(Stop *ptr);
  StopStats &AddLinkedBus(Bus *ptr);
  StopStats &SetLinkedStopDistance(Stop *ptr, double distance);
  Stop *stop_ptr{nullptr};
  std::unordered_set<Bus *> linked_buses{};
  std::unordered_map<Stop *, double> linked_stops{};
};

//! Wrapper for Bus, used to painlessly add extra information about route
struct BusStats {
  BusStats &SetBus(Bus *ptr);
  BusStats &SetTotalStops(size_t number);
  BusStats &SetUniqueStops(std::unordered_set<Stop *> &&stops);
  BusStats &SetDirectLength(double val);
  BusStats &SetRealLength(double val);
  Bus *bus_ptr{nullptr};
  size_t total_stops{};
  std::unordered_set<Stop *> unique_stops{};
  double direct_lenght{};
  double real_length{};
};

using BusStorage = std::unordered_map<std::string_view, BusStats>;
using StopStorage = std::unordered_map<std::string_view, StopStats>;

//! Iterator over geo::Coordinates extracted from DataStorage::Stop pointers
//! container
class StopCoordsIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = geo::Coordinates;
  using pointer = const geo::Coordinates *;
  using reference = const geo::Coordinates &;
  using wrapped_iterator = std::vector<const Stop *>::const_iterator;

  StopCoordsIterator() = default;
  explicit StopCoordsIterator(wrapped_iterator wrapped) : wrapped_(wrapped) {}

  reference operator*() const;
  pointer operator->() const;
  bool operator==(const StopCoordsIterator &other) const;
  bool operator!=(const StopCoordsIterator &other) const;
  StopCoordsIterator operator++(int);
  StopCoordsIterator &operator++();

private:
  wrapped_iterator wrapped_{};
};

//! Stores somehow logically connected routes and stops (used in MapRenderer)
struct RoutesData {
  std::vector<const Stop *> routes_stops;
  std::vector<const BusStats *> bus_stats;
};

} // namespace data

//! Format independent description of the fastest path
struct RouteAnswer {
  struct Wait {
    Wait &SetStop(const data::Stop *ptr);
    Wait &SetTime(double number);
    const data::Stop *stop;
    double time;
  };
  struct Bus {
    Bus &SetBus(const data::Bus *ptr);
    Bus &SetSpanCount(size_t number);
    Bus &SetTime(double number);
    const data::Bus *bus;
    size_t span_count;
    double time;
  };
  using Item = std::variant<Wait, Bus>;
  double total_time;
  std::vector<Item> items;
};

/*!
 * Represents real world entity DataStorage::Stop (and its properties) as
 * graph vertex. Combination of all class fields defines unique graph vertex.
 */
class Vertex {
public:
  Vertex &SetStop(const data::Stop *st);
  Vertex &SetWait(bool wait);
  const data::Stop *GetStop() const;
  bool GetWaitStatus() const;
  bool operator==(const Vertex &other) const;

private:
  const data::Stop *stop{nullptr};
  bool must_wait{false};
};

struct VertexHasher {
  size_t operator()(const Vertex &obj) const;
};
} // namespace core

namespace graphics {
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
};
} // namespace graphics