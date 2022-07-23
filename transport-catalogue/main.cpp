#include "domain.h"
#include "json_builder.h"
#include "json_reader.h"

int main() {
  TransportCatalogue database{};
  MapRenderer renderer{};
  TransportRouter router{database};
  RequestHandler req_handler{std::cout, database, renderer, router};
  JsonReader json_reader{std::cin, database, req_handler};
  json_reader.ProcessInput(OutputFormat::Json{});
}