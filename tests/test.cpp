#define BOOST_TEST_MODULE "Unit tests"

#include "../transport-catalogue/domain.h"
#include "../transport-catalogue/json.h"
#include "../transport-catalogue/json_reader.h"
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <fstream>
#include <optional>
#include <string>

std::optional<double> FindTime(int req_id, const io::json::Array &source) {
  for (const auto &el : source) {
    const auto &el_dict = el.AsMap();
    if (el_dict.count("request_id")) {
      int other_req_id = el_dict.at("request_id").AsInt();
      if (other_req_id == req_id) {
        if (el_dict.count("total_time")) {
          return el_dict.at("total_time").AsDouble();
        }
      }
    }
  }
  return std::nullopt;
}

BOOST_AUTO_TEST_CASE(total_time_test) {
  std::ifstream input_data_json_file, correct_output_json_file;
  std::ostringstream out_str_stream;
  std::string curr_dir = CURR_TEST_DIR;

  input_data_json_file.open(curr_dir + "/timetest_input.json");
  correct_output_json_file.open(curr_dir + "/timetest_output.json");

  core::TransportCatalogue database{};
  core::TransportRouter router{database};
  graphics::MapRenderer renderer{};
  io::RequestHandler req_handler{out_str_stream, database, renderer, router};
  io::JsonReader json_reader{database, req_handler};

  const io::json::Document doc = io::json::Load(input_data_json_file);
  const auto &doc_map = doc.GetRoot().AsMap();

  json_reader.ProcessInput(doc_map);
  std::istringstream questionable_output_json{out_str_stream.str()};

  const auto json_correct = io::json::Load(correct_output_json_file);
  const auto json_testing = io::json::Load(questionable_output_json);

  BOOST_REQUIRE(json_testing.GetRoot().IsArray());

  const auto &corr_arr = json_correct.GetRoot().AsArray();
  const auto &test_arr = json_testing.GetRoot().AsArray();

  for (const auto &el : corr_arr) {
    const auto &el_dict = el.AsMap();
    if (el_dict.count("total_time") && el_dict.count("request_id")) {
      int req_id = el_dict.at("request_id").AsInt();
      double corr_time = el_dict.at("total_time").AsDouble();
      auto other_time = FindTime(req_id, test_arr);
      BOOST_REQUIRE_MESSAGE(other_time.has_value(),
                            "request_id " << req_id
                                          << " not found or has no total_time");
      BOOST_REQUIRE_MESSAGE(std::abs(corr_time - other_time.value()) < 1e-8,
                            "request_id " << req_id << " total_time mismatch");
    }
  }
}