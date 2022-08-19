/*!
 * \file svg.h
 * \brief Scalable Vector Graphics (SVG) generation library
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

//! The main and only SVG generation library namespace
namespace graphics::svg {

/*!
 * Represents <a
 * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Positions">point
 * on grid</a>
 */
struct Point {
  Point() = default;
  Point(double xc, double yc) : x(xc), y(yc) {}
  double x = 0;
  double y = 0;
};

/*!
 * Defines <a
 * href="https://developer.mozilla.org/en-US/docs/Web/CSS/color_value/rgba">color</a>
 */
struct Rgb {
  Rgb() = default;
  Rgb(uint8_t r, uint8_t g, uint8_t b) : red{r}, green{g}, blue{b} {}
  uint8_t red{0};
  uint8_t green{0};
  uint8_t blue{0};
};

/*!
 * Defines <a
 * href="https://developer.mozilla.org/en-US/docs/Web/CSS/color_value/rgba">color</a>
 */
struct Rgba {
  Rgba() = default;
  Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
      : red{r}, green{g}, blue{b}, opacity{a} {}
  uint8_t red{0};
  uint8_t green{0};
  uint8_t blue{0};
  double opacity{1.0};
};

/*!
 * Represents <a
 * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/stroke-linecap">stroke-linecap</a>
 */
enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

/*!
 * Represents <a
 * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/stroke-linejoin">stroke-linejoin</a>
 */
enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

/*!
 * Maps StrokeLineCap value to string while outputting to std::ostream
 */
const std::unordered_map<StrokeLineCap, std::string> LineCapStr = {
    {StrokeLineCap::BUTT, "butt"},
    {StrokeLineCap::ROUND, "round"},
    {StrokeLineCap::SQUARE, "square"},
};

/*!
 * Maps StrokeLineJoin value to string while outputting to std::ostream
 */
const std::unordered_map<StrokeLineJoin, std::string> LineJoinStr = {
    {StrokeLineJoin::MITER, "miter"},
    {StrokeLineJoin::MITER_CLIP, "miter-clip"},
    {StrokeLineJoin::ROUND, "round"},
    {StrokeLineJoin::BEVEL, "bevel"},
    {StrokeLineJoin::ARCS, "arcs"},
};

/*!
 * Symbols to be replaced in Text content
 */
const std::unordered_map<char, std::string> SymbolsToReplace = {
    {'\"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"},
    {'>', "&gt;"},    {'&', "&amp;"},
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
const Color NoneColor{"none"};

std::ostream &operator<<(std::ostream &out, const StrokeLineCap &linecap);
std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &linejoin);
std::ostream &operator<<(std::ostream &out, const svg::Rgb &rgb);
std::ostream &operator<<(std::ostream &out, const svg::Rgba &rgba);
std::ostream &operator<<(std::ostream &out, const svg::Color &color);

//! Used to store and print indentation in render calls
struct RenderContext {
  RenderContext(std::ostream &outs) : out(outs) {}
  RenderContext(std::ostream &outs, int ind_step, int ind = 0)
      : out(outs), indent_step(ind_step), indent(ind) {}

  RenderContext Indented() const;
  void RenderIndent() const;

  std::ostream &out;
  int indent_step{2};
  int indent{0};
};

/*!
 * Interface of any SVG object
 */
class Object {
public:
  virtual void Render(const RenderContext &context) const;
  virtual ~Object() = default;

private:
  virtual void RenderObject(const RenderContext &context) const = 0;
};

/*!
 *  Abstract class, that is used as interface of objects container
 *  (not necessarily related to SVG)
 */
class ObjectContainer {
public:
  template <typename T> void Add(T obj);
  virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;
  virtual ~ObjectContainer() = default;

protected:
  std::vector<std::unique_ptr<Object>> objects_;
};

/*!
 * Interface of drawable object (not necessarily related to SVG)
 */
class Drawable {
public:
  virtual void Draw(ObjectContainer &container) const = 0;
  virtual ~Drawable() = default;
};

/*!
 * Used to convert various color types into string (via std::visit)
 * while outputting to std::ostream
 */
template <typename OutStream> struct ColorPrinter {
public:
  explicit ColorPrinter(OutStream &out) : out_{out} {}
  void operator()(std::monostate) { out_ << NoneColor; }
  void operator()(std::string color) { out_ << color; }
  void operator()(svg::Rgb color) { out_ << color; }
  void operator()(svg::Rgba color) { out_ << color; }

private:
  OutStream &out_;
};

/*!
 * Used to set, store and print various fill & stroke properties
 */
template <typename Owner> class PathProps {
public:
  Owner &SetFillColor(Color color);
  Owner &SetStrokeColor(Color color);
  Owner &SetStrokeWidth(double width);
  Owner &SetStrokeLineCap(StrokeLineCap line_cap);
  Owner &SetStrokeLineJoin(StrokeLineJoin line_join);

protected:
  ~PathProps() = default;
  void RenderAttrs(std::ostream &out) const;

private:
  std::optional<Color> fill_color_ = std::nullopt;
  std::optional<Color> stroke_color_ = std::nullopt;
  std::optional<double> stroke_width_ = std::nullopt;
  std::optional<StrokeLineCap> stroke_linecap_ = std::nullopt;
  std::optional<StrokeLineJoin> stroke_linejoin_ = std::nullopt;

  Owner &AsOwner();
};

/*!
 * Represents <a
 * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle">circle</a>
 */
class Circle final : public Object, public PathProps<Circle> {
public:
  Circle &SetCenter(Point center);
  Circle &SetRadius(double radius);

private:
  void RenderObject(const RenderContext &context) const override;

  Point center_;
  double radius_ = 1.0;
};

/*!
 * Represents <a
 * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline">polyline</a>
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
  Polyline &AddPoint(Point point);

private:
  std::vector<Point> points_;
  void RenderObject(const RenderContext &context) const override;
};

/*!
 * Represents <a
 * href="https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text">text</a>
 */
class Text final : public Object, public PathProps<Text> {
public:
  Text &SetPosition(Point pos);
  Text &SetOffset(Point offset);
  Text &SetFontSize(uint32_t size);
  Text &SetFontFamily(std::string font_family);
  Text &SetFontWeight(std::string font_weight);
  Text &SetData(std::string data);

private:
  Point position_{0, 0};
  Point displacement_{0, 0};
  uint32_t font_size_{1};
  std::optional<std::string> font_family_{};
  std::optional<std::string> font_weight_{};
  std::string data_{};

  void RenderObject(const RenderContext &context) const override;
};

/*!
 * Represents final SVG picture, aka composition containing all
 * previously created objects
 */
class Document : public ObjectContainer {
public:
  void AddPtr(std::unique_ptr<Object> &&obj);
  void Render(std::ostream &out) const;
};

template <typename T> void ObjectContainer::Add(T obj) {
  objects_.emplace_back(std::make_unique<T>(std::move(obj)));
}
template <typename Owner> Owner &PathProps<Owner>::SetFillColor(Color color) {
  fill_color_ = std::move(color);
  return AsOwner();
}
template <typename Owner> Owner &PathProps<Owner>::SetStrokeColor(Color color) {
  stroke_color_ = std::move(color);
  return AsOwner();
}
template <typename Owner>
Owner &PathProps<Owner>::SetStrokeWidth(double width) {
  stroke_width_ = width;
  return AsOwner();
}
template <typename Owner>
Owner &PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
  stroke_linecap_ = line_cap;
  return AsOwner();
}
template <typename Owner>
Owner &PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
  stroke_linejoin_ = line_join;
  return AsOwner();
}
template <typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream &out) const {
  using namespace std::literals;
  if (fill_color_) {
    out << " fill=\""sv << *fill_color_ << "\""sv;
  }
  if (stroke_color_) {
    out << " stroke=\""sv << *stroke_color_ << "\""sv;
  }
  if (stroke_width_) {
    out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
  }
  if (stroke_linecap_) {
    out << " stroke-linecap=\""sv << LineCapStr.at(*stroke_linecap_) << "\""sv;
  }
  if (stroke_linejoin_) {
    out << " stroke-linejoin=\""sv << LineJoinStr.at(*stroke_linejoin_)
        << "\""sv;
  }
}
template <typename Owner> Owner &PathProps<Owner>::AsOwner() {
  return static_cast<Owner &>(*this);
}

} // namespace graphics::svg