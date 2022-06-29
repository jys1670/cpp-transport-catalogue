#include "json_reader.h"

void JsonReader::ProcessInput(OutputFormat::Json) {
  const json::Document doc = json::Load(init.source);
  for (const auto &node : doc.GetRoot().AsMap().at("base_requests").AsArray()) {
    init.inputr.ParseCommand(node);
  }
  init.inputr.InsertAllIntoStorage();
  init.renderer.LoadSettings(doc.GetRoot().AsMap().at("render_settings"));
  for (const auto &node : doc.GetRoot().AsMap().at("stat_requests").AsArray()) {
    init.statr.ParseCommand(node);
  }
  init.statr.PrintAllStats(OutputFormat::Json{});
}