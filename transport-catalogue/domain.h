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

//! Dummy type used to specify program output format
struct OutputFormat {
  struct Json {};
  struct Xml {};
};

//! Cheap to copy objects used for information transfer to TransportCatalogue
namespace InputInfo {

//! %Stop name and position (transfer)
struct Stop {
  std::string_view name;
  geo::Coordinates pos;
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
} // namespace InputInfo

//! Objects solely used for information storage (mainly in TransportCatalogue)
namespace DataStorage {

//! %Stop name and position (storage)
struct Stop {
  std::string name;
  geo::Coordinates pos;
};

//! Route name and stops (storage)
struct Bus {
  std::string name;
  std::vector<Stop *> stops;
  bool is_circular;
};

//! Wrapper for Stop, used to painlessly add extra information about stop
struct StopStats {
  Stop *stop_ptr;
  std::unordered_set<Bus *> linked_buses;
  std::unordered_map<Stop *, double> linked_stops;
};

//! Wrapper for Bus, used to painlessly add extra information about route
struct BusStats {
  Bus *bus_ptr;
  size_t total_stops;
  std::unordered_set<Stop *> unique_stops;
  double direct_lenght;
  double real_length;
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

} // namespace DataStorage