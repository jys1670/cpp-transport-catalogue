/*!
\file json.h
 \brief JSON read/write library
*/
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace json {

class Node;
class Document;

class ParsingError : public std::runtime_error {
public:
  using runtime_error::runtime_error;
};

struct PrintContext {
  std::ostream &out;
  int indent_step = 4;
  int indent = 0;

  void PrintIndent() const;
  PrintContext Indented() const;
};

bool operator==(const Node &lhs, const Node &rhs);
bool operator!=(const Node &lhs, const Node &rhs);

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
const std::unordered_set<char> Delimiters = {' ', '\n', '\t', '\r'};
const std::unordered_map<char, std::string> CharsToReplace = {
    {'\n', R"(\n)"},
    {'\r', R"(\r)"},
    {'\"', R"(\")"},
    {'\\', R"(\\)"},
};

Document Load(std::istream &input);

template <typename Value>
void PrintValue(const Value &value, const PrintContext &ctx) {
  ctx.out << value;
}
void PrintValue(std::nullptr_t, const PrintContext &ctx);
void PrintValue(const Array &arr, const PrintContext &ctx);
void PrintValue(const Dict &dict, const PrintContext &ctx);
void PrintValue(const std::string &str, const PrintContext &ctx);
void PrintValue(const bool &val, const PrintContext &ctx);
void PrintNode(const Node &node, const PrintContext &ctx);
void Print(const Document &doc, std::ostream &output);

class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int,
                                        double, std::string> {
public:
  using Value =
      std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
  using variant::variant;

  Node(const Document &doc);

  bool IsInt() const;
  bool IsDouble() const;
  bool IsPureDouble() const;
  bool IsBool() const;
  bool IsString() const;
  bool IsNull() const;
  bool IsArray() const;
  bool IsMap() const;

  int AsInt() const;
  bool AsBool() const;
  double AsDouble() const;
  const std::string &AsString() const;
  const Array &AsArray() const;
  const Dict &AsMap() const;

  const Value &GetValue() const;
};

class Document {
public:
  explicit Document(Node root);

  const Node &GetRoot() const;

private:
  Node root_;
};

} // namespace json