#include "domain.h"
#include "json_builder.h"
#include "json_reader.h"
#include "serialization.h"

#include <filesystem>
#include <fstream>

void PrintUsage(const std::string &filename, std::ostream &stream = std::cerr) {
  stream << "Usage: " << filename << " [make_base|process_requests]\n";
}

int main(int argc, char *argv[]) {
  std::filesystem::path fp{argv[0]};
  if (argc != 2) {
    PrintUsage(fp.filename().string());
    return 1;
  }
  const std::string_view mode(argv[1]);

  core::TransportCatalogue database{};
  core::TransportRouter router{database};
  graphics::MapRenderer renderer{};
  core::RequestHandler req_handler{std::cout, database, renderer, router};
  json::JsonReader json_reader{database, req_handler};

  const json::Document doc = json::Load(std::cin);
  const auto &doc_map = doc.GetRoot().AsMap();

  if (mode == "make_base") {
    serialization::Serializer serializer{};

    json_reader.ProcessInput(doc_map, input_info::OutputFormat::None);
    database.ExportDataBase(serializer);
    renderer.ExportRenderSettings(serializer);
    router.ExportState(serializer);

    std::ofstream out(
        doc_map.at("serialization_settings").AsMap().at("file").AsString());
    serializer.SerializeToOstream(&out);

  } else if (mode == "process_requests") {
    std::ifstream in(
        doc_map.at("serialization_settings").AsMap().at("file").AsString());

    serialization::TrCatalogue sr_catalogue;
    sr_catalogue.ParseFromIstream(&in);

    database.ImportDataBase(sr_catalogue);
    renderer.ImportRenderSettings(sr_catalogue);
    router.ImportState(sr_catalogue);

    json_reader.ProcessInput(doc_map);
  } else {
    PrintUsage(fp.filename().string());
    return 1;
  }
}