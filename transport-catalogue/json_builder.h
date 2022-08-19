/*!
 * \file json_builder.h
 * \brief Handy JSON generator using method chaining approach
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "json.h"

namespace io::json {

//! Handy type used to construct JSON via method chaining approach
class Builder {
public:
  struct ValueContext;
  struct ArrayItemContext;
  struct MapKeyContext;
  struct MapItemContext;

  //! Stores common Builder for all available states
  struct Context {
  public:
    explicit Context(Builder *builder);

    Builder *builder_;
  };

  //! Represents state in which all Builder actions are allowed
  struct ValueContext : Context {
    explicit ValueContext(Builder *builder);

    ValueContext Value(Node::Value val);
    MapKeyContext Key(std::string key);

    MapItemContext StartDict();
    ValueContext EndDict();

    ArrayItemContext StartArray();
    ValueContext EndArray();

    Node Build();
  };

  //! Actions that are allowed after setting dictionary key
  struct MapKeyContext : Context {
    explicit MapKeyContext(Builder *builder);

    MapItemContext Value(Node::Value val);
    MapItemContext StartDict();
    ArrayItemContext StartArray();
  };

  //! Actions that are allowed after insertion of dictionary {key, value} pair
  struct MapItemContext : Context {
    explicit MapItemContext(Builder *builder);
    explicit MapItemContext(Context ctx);

    MapKeyContext Key(std::string key);
    ValueContext EndDict();
  };

  //! Actions that are allowed inside of an array
  struct ArrayItemContext : Context {
    explicit ArrayItemContext(Builder *builder);
    explicit ArrayItemContext(Context ctx);

    ArrayItemContext Value(Node::Value val);
    MapItemContext StartDict();
    ArrayItemContext StartArray();
    ValueContext EndArray();
  };

  Builder() = default;

  ValueContext Value(Node::Value val);
  MapKeyContext Key(std::string key);
  MapItemContext StartDict();
  ArrayItemContext StartArray();
  ValueContext EndDict();
  ValueContext EndArray();

  Node Build();

private:
  bool builder_empty_{true};
  std::optional<std::string> curr_map_key_{};
  Node root_{};
  std::vector<Node *> nodes_stack_{};
};

} // namespace io::json
