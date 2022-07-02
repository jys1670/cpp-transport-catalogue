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

//! The main and only JSON read/write library namespace
namespace json {

class Node;
class Document;

class ParsingError : public std::runtime_error {
public:
  using runtime_error::runtime_error;
};

//! Used to store and print indentation in recursive print calls
struct PrintContext {
  std::ostream &out;
  int indent_step = 4;
  int indent = 0;

  void PrintIndent() const;
  PrintContext Indented() const;
};

bool operator==(const Node &lhs, const Node &rhs);
bool operator!=(const Node &lhs, const Node &rhs);

/*!
 * \details Type that is widely used to define various settings,
 * with option names (strings) stored as dictionary keys
 */
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

//! Characters treated as delimiters when reading from input stream
const std::unordered_set<char> Delimiters = {' ', '\n', '\t', '\r'};

//! Characters replaced by PrintValue(const std::string &str, const PrintContext &ctx) during printing
const std::unordered_map<char, std::string> CharsToReplace = {
    {'\n', R"(\n)"},
    {'\r', R"(\r)"},
    {'\"', R"(\")"},
    {'\\', R"(\\)"},
};

Document Load(std::istream &input);

/**
    \fn PrintValue(const Value &value, const PrintContext &ctx)
    \details Prints whatever type with defined operator<<(std::basic_ostream) to ctx.out
 */
template <typename Value>
void PrintValue(const Value &value, const PrintContext &ctx) {
  ctx.out << value;
}

/**
    \fn PrintValue(std::nullptr_t, const PrintContext &ctx)
    \details Prints "null" (string) to ctx.out
 */
void PrintValue(std::nullptr_t, const PrintContext &ctx);

/**
    \fn PrintValue(const Array &arr, const PrintContext &ctx)
    \details Calls PrintNode() for array elements
 */
void PrintValue(const Array &arr, const PrintContext &ctx);

/**
    \fn PrintValue(const Dict &dict, const PrintContext &ctx)
    \details Prints dictionary keys and calls PrintNode() for values
 */
void PrintValue(const Dict &dict, const PrintContext &ctx);

/**
    \fn PrintValue(const std::string &str, const PrintContext &ctx)
    \details Prints given string to ctx.out
 */
void PrintValue(const std::string &str, const PrintContext &ctx);

/**
    \fn PrintValue(const bool &bl, const PrintContext &ctx)
    \details Prints "true" or "false" to ctx.out
 */
void PrintValue(const bool &bl, const PrintContext &ctx);

//! \details Detects value type of node and calls appropriate print functions, could be recursive
void PrintNode(const Node &node, const PrintContext &ctx);

//! Wrapper for PrintNode() that works with Document
void Print(const Document &doc, std::ostream &output);

//! Runtime polymorphic type representing <a href="https://en.wikipedia.org/wiki/JSON#Data_types">JSON basic data types</a>
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

//! Wrapper for Node, which stores Node containing entire input data
class Document {
public:
  explicit Document(Node root);

  const Node &GetRoot() const;

private:
  Node root_;
};

} // namespace json