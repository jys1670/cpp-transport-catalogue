#include "geo.h"

bool geo::Coordinates::operator!=(const Coordinates &other) const {
  return !(*this == other);
}

bool geo::Coordinates::operator==(const Coordinates &other) const {
  return lat == other.lat && lng == other.lng;
}
