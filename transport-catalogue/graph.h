/*!
 * \file graph.h
 * \brief Directed weighted graphs declarations and definitions
 */

#pragma once

#include <cstdlib>
#include <iterator>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace graph {

template <typename It> class Range {
public:
  using ValueType = typename std::iterator_traits<It>::value_type;

  Range(It begin, It end) : begin_(begin), end_(end) {}
  It begin() const { return begin_; }
  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

template <typename C> auto AsRange(const C &container) {
  return Range{container.begin(), container.end()};
}

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight> struct Edge {
  Edge &SetFromVertex(VertexId vert) {
    from = vert;
    return *this;
  }
  Edge &SetToVertex(VertexId vert) {
    to = vert;
    return *this;
  }
  Edge &SetWeight(Weight value) {
    weight = value;
    return *this;
  }
  Edge &SetBus(const core::data::Bus *ptr) {
    bus = ptr;
    return *this;
  }
  Edge &SetStopCount(size_t count) {
    stop_count = count;
    return *this;
  }
  VertexId from;
  VertexId to;
  Weight weight;
  const core::data::Bus *bus;
  size_t stop_count;
};

template <typename Weight> class DirectedWeightedGraph {
public:
  using IncidenceList = std::vector<EdgeId>;
  using IncidentEdgesRange = Range<typename IncidenceList::const_iterator>;

  DirectedWeightedGraph() = default;
  explicit DirectedWeightedGraph(size_t vertex_count);
  EdgeId AddEdge(const Edge<Weight> &edge);

  size_t GetVertexCount() const;
  size_t GetEdgeCount() const;
  const Edge<Weight> &GetEdge(EdgeId edge_id) const;
  IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

  DirectedWeightedGraph &SetEdges(std::vector<Edge<Weight>> &&edges) {
    edges_ = std::move(edges);
    return *this;
  }
  DirectedWeightedGraph &SetIncidenceLists(std::vector<IncidenceList> &&lists) {
    incidence_lists_ = std::move(lists);
    return *this;
  }

private:
  std::vector<Edge<Weight>> edges_;
  std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight> &edge) {
  edges_.push_back(edge);
  const EdgeId id = edges_.size() - 1;
  incidence_lists_.at(edge.from).push_back(id);
  return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
  return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
  return edges_.size();
}

template <typename Weight>
const Edge<Weight> &
DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
  return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
  return AsRange(incidence_lists_.at(vertex));
}
} // namespace graph