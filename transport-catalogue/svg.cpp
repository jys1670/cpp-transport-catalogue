#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext &context) const {
  context.out << std::endl;
  context.RenderIndent();
  RenderObject(context);
}

// ---------- Circle ------------------

Circle &Circle::SetCenter(Point center) {
  center_ = center;
  return *this;
}

Circle &Circle::SetRadius(double radius) {
  radius_ = radius;
  return *this;
}

void Circle::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << '"';
  out << R"( r=")" << radius_ << '"';
  RenderAttrs(out);
  out << R"(/>)";
}

// ---------- Polyline ------------------

Polyline &Polyline::AddPoint(Point point) {
  points_.emplace_back(point);
  return *this;
}

void Polyline::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << R"(<polyline points=")";
  bool first = true;
  for (auto &point : points_) {
    if (first) {
      first = false;
    } else {
      out << ' ';
    }
    out << point.x << ',' << point.y;
  }
  out << '"';
  RenderAttrs(out);
  out << R"(/>)";
}

// ---------- Text ------------------

Text &Text::SetPosition(Point pos) {
  position_ = pos;
  return *this;
}

Text &Text::SetOffset(Point offset) {
  displacement_ = offset;
  return *this;
}

Text &Text::SetFontSize(uint32_t size) {
  font_size_ = size;
  return *this;
}

Text &Text::SetFontFamily(std::string font_family) {
  font_family_ = std::move(font_family);
  return *this;
}

Text &Text::SetFontWeight(std::string font_weight) {
  font_weight_ = std::move(font_weight);
  return *this;
}

Text &Text::SetData(std::string data) {
  data_ = std::move(data);
  return *this;
}

void Text::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << R"(<text)";
  out << R"( x=")" << position_.x << R"(" y=")" << position_.y << R"(" dx=")"
      << displacement_.x << R"(" dy=")" << displacement_.y << R"(" font-size=")"
      << font_size_ << '"';
  if (font_family_)
    out << R"( font-family=")" << *font_family_ << '"';
  if (font_weight_)
    out << R"( font-weight=")" << *font_weight_ << '"';
  RenderAttrs(out);
  out << '>';
  for (auto ch : data_) {
    if (SymbolsToReplace.count(ch)) {
      out << SymbolsToReplace.at(ch);
    } else {
      out << ch;
    }
  }
  out << R"(</text>)";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object> &&obj) {
  objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream &out) const {
  out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << '\n'
      << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";
  for (auto &obj : objects_) {
    obj->Render(RenderContext{out}.Indented());
  }
  out << '\n' << R"(</svg>)";
}

std::ostream &operator<<(std::ostream &out, const StrokeLineCap &linecap) {
  out << svg::LineCapStr.at(linecap);
  return out;
}
std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &linejoin) {
  out << svg::LineJoinStr.at(linejoin);
  return out;
}
std::ostream &operator<<(std::ostream &out, const Rgb &rgb) {
  out << "rgb(" << unsigned(rgb.red) << ',' << unsigned(rgb.green) << ','
      << unsigned(rgb.blue) << ')';
  return out;
}
std::ostream &operator<<(std::ostream &out, const Rgba &rgba) {
  out << "rgba(" << unsigned(rgba.red) << ',' << unsigned(rgba.green) << ','
      << unsigned(rgba.blue) << ',' << rgba.opacity << ')';
  return out;
}
std::ostream &operator<<(std::ostream &out, const Color &color) {
  std::visit(svg::ColorPrinter{out}, color);
  return out;
}
void RenderContext::RenderIndent() const {
  for (int i = 0; i < indent; ++i) {
    out.put(' ');
  }
}
RenderContext RenderContext::Indented() const {
  return {out, indent_step, indent + indent_step};
}
} // namespace svg