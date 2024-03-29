/*!
 * \file geo.h
 * \brief Geographic coordinates related elements
 */
#pragma once

#include <cmath>

//! Geographic coordinates related elements
namespace geo {

//! Point in geographic coordinate system (GCS)
struct Coordinates {
  double lat; //!< latitude
  double lng; //!< longitude
  bool operator==(const Coordinates &other) const;
  bool operator!=(const Coordinates &other) const;
};

//! Assuming Earth is ideal sphere, computes great-circle distance
inline double ComputeDistance(Coordinates from, Coordinates to) {
  if (from == to) {
    return 0;
  }
  static const double dr = 3.1415926535 / 180.;
  static const double earth_radius = 6371000;
  return acos(sin(from.lat * dr) * sin(to.lat * dr) +
              cos(from.lat * dr) * cos(to.lat * dr) *
                  cos(std::abs(from.lng - to.lng) * dr)) *
         earth_radius;
}

} // namespace geo
