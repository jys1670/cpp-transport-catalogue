#include "json_builder.h"

#include <iostream>
#include <utility>

namespace json {

/* Builder */
ValueContext Builder::Value(Node::Value val) {
  if (builder_empty_) {
    root_.GetValue() = std::move(val);
    builder_empty_ = false;
    return ValueContext{this};
  }
  if (nodes_stack_.empty()) {
    throw std::logic_error("JSON building has already been finished, "
                           "modifications are not permitted.");
  }
  auto &curr_node = *nodes_stack_.back();
  if (curr_node.IsMap()) {
    if (curr_map_key_.has_value()) {
      auto &dict_val =
          std::get<Dict>(curr_node.GetValue())[curr_map_key_.value()]
              .GetValue();
      dict_val = std::move(val);
    } else {
      throw std::logic_error(
          "Dictionary key must be declared before setting corresponding value");
    }
  } else if (curr_node.IsArray()) {
    auto &arr = std::get<Array>(curr_node.GetValue());
    arr.emplace_back(std::move(val));
  } else {
    throw std::logic_error(
        "Incorrect call chain order, Value(...) is not allowed.");
  }
  curr_map_key_.reset();
  return ValueContext{this};
}

MapKeyContext Builder::Key(std::string key) {
  if (builder_empty_) {
    throw std::logic_error("Incorrect call chain order, found Key(...) without "
                           "corresponding Dict(...)");
  }
  if (nodes_stack_.empty()) {
    throw std::logic_error("JSON building has already been finished, "
                           "modifications are not permitted.");
  }
  if (!(nodes_stack_.back()->IsMap())) {
    throw std::logic_error("Incorrect call chain order, found Key(...) without "
                           "corresponding Dict(...)");
  }
  if (curr_map_key_.has_value()) {
    throw std::logic_error("Previously called Key(...) must contain value "
                           "before calling Key(...) again");
  }
  curr_map_key_ = std::move(key);
  return MapKeyContext{this};
}

MapItemContext Builder::StartDict() {
  Dict dict;
  if (builder_empty_) {
    nodes_stack_.emplace_back(&(root_ = std::move(dict)));
    builder_empty_ = false;
    return MapItemContext{this};
  }
  if (nodes_stack_.empty()) {
    throw std::logic_error("JSON building has already been finished, "
                           "modifications are not permitted.");
  }
  auto &curr_node = *nodes_stack_.back();
  if (curr_node.IsMap()) {
    if (curr_map_key_.has_value()) {
      nodes_stack_.emplace_back(
          &(std::get<Dict>(curr_node.GetValue())[curr_map_key_.value()] =
                std::move(dict)));
      curr_map_key_.reset();
    } else {
      throw std::logic_error(
          "Dictionary key must be declared before setting corresponding value");
    }
  } else if (curr_node.IsArray()) {
    nodes_stack_.emplace_back(
        &(std::get<Array>(curr_node.GetValue()).emplace_back(std::move(dict))));
  } else {
    throw std::logic_error(
        "Incorrect call chain order, StartDict(...) is not allowed.");
  }
  return MapItemContext{this};
}

ArrayItemContext Builder::StartArray() {
  Array array;
  if (builder_empty_) {
    nodes_stack_.emplace_back(&(root_ = std::move(array)));
    builder_empty_ = false;
    return ArrayItemContext{this};
  }
  if (nodes_stack_.empty()) {
    throw std::logic_error("JSON building has already been finished, "
                           "modifications are not permitted.");
  }
  auto &curr_node = *nodes_stack_.back();
  if (curr_node.IsMap()) {
    if (curr_map_key_.has_value()) {
      nodes_stack_.emplace_back(
          &(std::get<Dict>(curr_node.GetValue())[curr_map_key_.value()] =
                std::move(array)));
      curr_map_key_.reset();
    } else {
      throw std::logic_error(
          "Dictionary key must be declared before setting corresponding value");
    }
  } else if (curr_node.IsArray()) {
    nodes_stack_.emplace_back(&(
        std::get<Array>(curr_node.GetValue()).emplace_back(std::move(array))));
  } else {
    throw std::logic_error(
        "Incorrect call chain order, StartArray(...) is not allowed.");
  }
  return ArrayItemContext{this};
}

ValueContext Builder::EndDict() {
  if (builder_empty_) {
    throw std::logic_error(
        "Incorrect call chain order, found EndDict(...) without "
        "corresponding Dict(...)");
  }
  if (nodes_stack_.empty()) {
    throw std::logic_error("JSON building has already been finished, "
                           "modifications are not permitted.");
  }
  if (!(nodes_stack_.back()->IsMap())) {
    throw std::logic_error(
        "Incorrect call chain order, found EndDict(...) without "
        "corresponding Dict(...)");
  }
  if (curr_map_key_.has_value()) {
    throw std::logic_error("Previously called Key(...) must contain value "
                           "before calling Key(...) again");
  }
  nodes_stack_.pop_back();
  return ValueContext{this};
}

ValueContext Builder::EndArray() {
  if (builder_empty_) {
    throw std::logic_error(
        "Incorrect call chain order, found EndArray(...) without "
        "corresponding Array(...)");
  }
  if (nodes_stack_.empty()) {
    throw std::logic_error("JSON building has already been finished, "
                           "modifications are not permitted.");
  }
  if (!(nodes_stack_.back()->IsArray())) {
    throw std::logic_error(
        "Incorrect call chain order, found EndArray(...) without "
        "corresponding Array(...)");
  }
  nodes_stack_.pop_back();
  return ValueContext{this};
}

Node Builder::Build() {
  if (builder_empty_) {
    throw std::logic_error(
        "Calling Build(...) with no JSON elements defined is not allowed");
  }
  if (!nodes_stack_.empty()) {
    throw std::logic_error("Incorrect call chain order, found Build(...) while "
                           "array or dictionary construction is in progress");
  }
  return root_;
}

/* Context */
Context::Context(Builder *builder) : builder_{builder} {}

/* ValueContext */
ValueContext::ValueContext(Builder *builder) : Context{builder} {}

MapKeyContext ValueContext::Key(std::string key) {
  return builder_->Key(std::move(key));
}

ValueContext ValueContext::Value(Node::Value val) {
  return builder_->Value(std::move(val));
}

MapItemContext ValueContext::StartDict() { return builder_->StartDict(); }

ValueContext ValueContext::EndDict() { return builder_->EndDict(); }

ArrayItemContext ValueContext::StartArray() { return builder_->StartArray(); }

ValueContext ValueContext::EndArray() { return builder_->EndArray(); }

Node ValueContext::Build() { return builder_->Build(); }

/* MapKeyContext */
MapKeyContext::MapKeyContext(Builder *builder) : Context{builder} {}

MapItemContext MapKeyContext::Value(Node::Value val) {
  return MapItemContext{builder_->Value(std::move(val))};
}

MapItemContext MapKeyContext::StartDict() { return builder_->StartDict(); }

ArrayItemContext MapKeyContext::StartArray() { return builder_->StartArray(); }

/* MapItemContext */
MapItemContext::MapItemContext(Builder *builder) : Context{builder} {}

MapItemContext::MapItemContext(Context ctx) : Context{ctx.builder_} {}

MapKeyContext MapItemContext::Key(std::string key) {
  return builder_->Key(std::move(key));
}

ValueContext MapItemContext::EndDict() { return builder_->EndDict(); }

/* ArrayItemContext */
ArrayItemContext::ArrayItemContext(Builder *builder) : Context{builder} {}

ArrayItemContext::ArrayItemContext(Context ctx) : Context{ctx.builder_} {}

ArrayItemContext ArrayItemContext::Value(Node::Value val) {
  return ArrayItemContext{builder_->Value(std::move(val))};
}

MapItemContext ArrayItemContext::StartDict() { return builder_->StartDict(); }

ArrayItemContext ArrayItemContext::StartArray() {
  return builder_->StartArray();
}

ValueContext ArrayItemContext::EndArray() { return builder_->EndArray(); }

} // namespace json
