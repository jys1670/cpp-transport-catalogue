#include <cmath>
#include <limits>

#include "json.h"

namespace json {

bool operator==(const Node &lhs, const Node &rhs) {
  if (lhs.IsNull() && rhs.IsNull()) {
    return true;
  }
  if (lhs.IsInt() && rhs.IsInt()) {
    return lhs.AsInt() == rhs.AsInt();
  }
  if (lhs.IsPureDouble() && rhs.IsPureDouble()) {
    return std::fabs(lhs.AsDouble() - rhs.AsDouble()) <=
           std::numeric_limits<double>::epsilon();
  }
  if (lhs.IsBool() && rhs.IsBool()) {
    return lhs.AsBool() == rhs.AsBool();
  }
  if (lhs.IsString() && rhs.IsString()) {
    return lhs.AsString() == rhs.AsString();
  }
  if (lhs.IsArray() && rhs.IsArray()) {
    auto &v1 = lhs.AsArray(), &v2 = rhs.AsArray();
    return v1.size() == v2.size() &&
           std::equal(v1.begin(), v1.end(), v2.begin());
  }
  if (lhs.IsMap() && rhs.IsMap()) {
    auto &m1 = lhs.AsMap(), &m2 = rhs.AsMap();
    return m1.size() == m2.size() &&
           std::equal(m1.begin(), m1.end(), m2.begin());
  }
  return false;
}

bool operator!=(const Node &lhs, const Node &rhs) { return !(lhs == rhs); }

namespace {

bool SequenceCheck(std::istream &input, const std::string &str) {
  return std::equal(str.begin(), str.end(),
                    std::istreambuf_iterator<char>(input));
}

Node LoadNode(std::istream &input);

Node LoadNumber(std::istream &input) {
  using namespace std::literals;

  std::string parsed_num;

  auto read_char = [&parsed_num, &input] {
    parsed_num += static_cast<char>(input.get());
    if (!input) {
      throw ParsingError("Failed to read number from stream"s);
    }
  };

  auto read_digits = [&input, read_char] {
    if (!std::isdigit(input.peek())) {
      throw ParsingError("A digit is expected"s);
    }
    while (std::isdigit(input.peek())) {
      read_char();
    }
  };

  if (input.peek() == '-') {
    read_char();
  }

  if (input.peek() == '0') {
    read_char();
  } else {
    read_digits();
  }

  bool is_int = true;
  if (input.peek() == '.') {
    read_char();
    read_digits();
    is_int = false;
  }

  if (int ch = input.peek(); ch == 'e' || ch == 'E') {
    read_char();
    if (ch = input.peek(); ch == '+' || ch == '-') {
      read_char();
    }
    read_digits();
    is_int = false;
  }

  try {
    if (is_int) {
      try {
        int result = std::stoi(parsed_num);
        return Node{result};
      } catch (...) {
      }
    }
    double result = std::stod(parsed_num);
    return Node{result};
  } catch (...) {
    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}

Node LoadString(std::istream &input) {
  using namespace std::literals;

  auto it = std::istreambuf_iterator<char>(input);
  auto end = std::istreambuf_iterator<char>();
  std::string s;
  while (true) {
    if (it == end) {
      throw ParsingError("String parsing error");
    }
    const char ch = *it;
    if (ch == '"') {
      ++it;
      break;
    } else if (ch == '\\') {
      ++it;
      if (it == end) {
        throw ParsingError("String parsing error");
      }
      const char escaped_char = *(it);
      switch (escaped_char) {
      case 'n':
        s.push_back('\n');
        break;
      case 't':
        s.push_back('\t');
        break;
      case 'r':
        s.push_back('\r');
        break;
      case '"':
        s.push_back('"');
        break;
      case '\\':
        s.push_back('\\');
        break;
      default:
        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
      }
    } else if (ch == '\n' || ch == '\r') {
      throw ParsingError("Unexpected end of line"s);
    } else {
      s.push_back(ch);
    }
    ++it;
  }

  return Node{s};
}

Node LoadArray(std::istream &input) {
  Array result;
  char c;
  for (; input >> c && c != ']';) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }
  if (!input && c != ']')
    throw ParsingError("Missing ]");

  return Node(std::move(result));
}

Node LoadDict(std::istream &input) {
  Dict result;
  char c;

  for (; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    std::string key = LoadString(input).AsString();
    input >> c;
    result.insert({std::move(key), LoadNode(input)});
  }
  if (!input && c != '}')
    throw ParsingError("Missing }");

  return Node(std::move(result));
}

Node LoadNode(std::istream &input) {
  char c{};
  for (; input >> c && Delimiters.count(c);)
    ;
  if (!input)
    throw ParsingError("Stream end reached - no data found");
  if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else if (c == 'n' && SequenceCheck(input, "ull")) {
    return Node{};
  } else if ((c == 't' && SequenceCheck(input, "rue")) ||
             (c == 'f' && SequenceCheck(input, "alse"))) {
    return Node{c == 't'};
  } else {
    input.putback(c);
    return LoadNumber(input);
  }
}

} // namespace

bool Node::IsInt() const { return std::holds_alternative<int>(*this); }
bool Node::IsDouble() const {
  return IsInt() || std::holds_alternative<double>(*this);
}
bool Node::IsPureDouble() const {
  return std::holds_alternative<double>(*this);
}
bool Node::IsBool() const { return std::holds_alternative<bool>(*this); }
bool Node::IsString() const {
  return std::holds_alternative<std::string>(*this);
}
bool Node::IsNull() const {
  return std::holds_alternative<std::nullptr_t>(*this);
}
bool Node::IsArray() const { return std::holds_alternative<Array>(*this); }
bool Node::IsMap() const { return std::holds_alternative<Dict>(*this); }

int Node::AsInt() const {
  if (!IsInt())
    throw std::logic_error("Incorrect type");
  return std::get<int>(*this);
}

bool Node::AsBool() const {
  if (!IsBool())
    throw std::logic_error("Incorrect type");
  return std::get<bool>(*this);
}

double Node::AsDouble() const {
  if (!IsDouble())
    throw std::logic_error("Incorrect type");
  if (IsInt())
    return std::get<int>(*this);
  return std::get<double>(*this);
}

const std::string &Node::AsString() const {
  if (!IsString())
    throw std::logic_error("Incorrect type");
  return std::get<std::string>(*this);
}

const Array &Node::AsArray() const {
  if (!IsArray())
    throw std::logic_error("Incorrect type");
  return std::get<Array>(*this);
}

const Dict &Node::AsMap() const {
  if (!IsMap())
    throw std::logic_error("Incorrect type");
  return std::get<Dict>(*this);
}

Node::Node(Node::Value &&val) { *this = std::move(val); }

Node::Node(const Document &doc) { *this = doc.GetRoot(); }

const Node::Value &Node::GetConstValue() const {
  return dynamic_cast<const Value &>(*this);
}

Node::Value &Node::GetValue() { return dynamic_cast<Value &>(*this); }

Document::Document(Node root) : root_(std::move(root)) {}

const Node &Document::GetRoot() const { return root_; }

Document Load(std::istream &input) { return Document{LoadNode(input)}; }

void Print(const Document &doc, std::ostream &output) {
  PrintNode(doc.GetRoot(), {output});
}

void PrintContext::PrintIndent() const {
  for (int i = 0; i < indent; ++i) {
    out.put(' ');
  }
}

PrintContext PrintContext::Indented() const {
  return {out, indent_step, indent_step + indent};
}

void PrintValue(std::nullptr_t, const PrintContext &ctx) { ctx.out << "null"; }

void PrintValue(const std::string &str, const PrintContext &ctx) {
  ctx.out << '\"';
  for (const auto ch : str) {
    if (CharsToReplace.count(ch)) {
      ctx.out << CharsToReplace.at(ch);
    } else {
      ctx.out << ch;
    }
  }
  ctx.out << '\"';
}

void PrintValue(const bool &bl, const PrintContext &ctx) {
  ctx.out << (bl ? "true" : "false");
}

void PrintNode(const Node &node, const PrintContext &ctx) {
  visit([&ctx](const auto &value) { PrintValue(value, ctx); },
        node.GetConstValue());
}

void PrintValue(const Array &arr, const PrintContext &ctx) {
  bool is_first{true};
  ctx.PrintIndent();
  ctx.out << "[\n";
  auto new_ctx = ctx.Indented();
  for (const auto &node : arr) {
    if (is_first) {
      is_first = false;
    } else {
      ctx.out << ",\n";
    }
    if (!node.IsArray() && !node.IsMap())
      new_ctx.PrintIndent();
    PrintNode(node, new_ctx);
  }
  ctx.out << '\n';
  ctx.PrintIndent();
  ctx.out << ']';
}

void PrintValue(const Dict &dict, const PrintContext &ctx) {
  bool is_first{true};
  ctx.PrintIndent();
  ctx.out << "{\n";
  auto new_ctx = ctx.Indented();
  for (const auto &[key, node] : dict) {
    if (is_first) {
      is_first = false;
    } else {
      ctx.out << ",\n";
    }
    new_ctx.PrintIndent();
    ctx.out << '"' << key << "\": ";
    if (node.IsArray() || node.IsMap())
      ctx.out << '\n';
    PrintNode(node, new_ctx);
  }
  ctx.out << '\n';
  ctx.PrintIndent();
  ctx.out << '}';
}

} // namespace json