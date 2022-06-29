#include "domain.h"

const geo::Coordinates &DataStorage::StopCoordsIterator::operator*() const {
  return (*wrapped_)->pos;
}

DataStorage::StopCoordsIterator::pointer
DataStorage::StopCoordsIterator::operator->() const {
  return &(*wrapped_)->pos;
}

bool DataStorage::StopCoordsIterator::operator==(
    const DataStorage::StopCoordsIterator &other) const {
  return wrapped_ == other.wrapped_;
}

bool DataStorage::StopCoordsIterator::operator!=(
    const DataStorage::StopCoordsIterator &other) const {
  return !(*this == other);
}

DataStorage::StopCoordsIterator
DataStorage::StopCoordsIterator::operator++(int) {
  auto tmp = *this;
  ++wrapped_;
  return tmp;
}

DataStorage::StopCoordsIterator &DataStorage::StopCoordsIterator::operator++() {
  ++wrapped_;
  return *this;
}
