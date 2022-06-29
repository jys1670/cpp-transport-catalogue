#pragma once

#include <iostream>

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

struct GenericReaderInit {
  GenericReaderInit(std::istream &input, std::ostream &output)
      : source{input}, outstream{output} {};
  std::istream &source;
  std::ostream &outstream;
  TransportCatalogue storage{};
  MapRenderer renderer{};
  InputReader inputr{storage};
  StatReader statr{storage, outstream, renderer};
};

class JsonReader {
public:
  explicit JsonReader(GenericReaderInit &input) : init{input} {};

  void ProcessInput(OutputFormat::Json);

private:
  GenericReaderInit &init;
};