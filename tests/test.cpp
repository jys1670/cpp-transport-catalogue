#define BOOST_TEST_MODULE "Unit tests"

#include <fstream>
#include <optional>
#include <cmath>
#include <string>
#include <boost/test/unit_test.hpp>
#include "../transport-catalogue/domain.h"
#include "../transport-catalogue/json.h"
#include "../transport-catalogue/json_reader.h"

std::optional<double> FindTime(int req_id, const json::Array& source) {
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

BOOST_AUTO_TEST_CASE( total_time_test )
{
  std::ifstream input_data_json_file, correct_output_json_file;
  std::ostringstream out_str_stream;
  std::string curr_dir = CURR_TEST_DIR;

  input_data_json_file.open(curr_dir + "/timetest_input.json");
  correct_output_json_file.open(curr_dir + "/timetest_output.json");

  TransportCatalogue database {};
  MapRenderer renderer {};
  TransportRouter router {database};
  RequestHandler req_handler {out_str_stream, database, renderer, router};
  JsonReader json_file_reader {input_data_json_file, database, req_handler};

  json_file_reader.ProcessInput(OutputFormat::Json{});
  std::istringstream questionable_output_json {out_str_stream.str()};

  const auto json_correct = json::Load(correct_output_json_file);
  const auto json_testing = json::Load(questionable_output_json);

  BOOST_REQUIRE( json_testing.GetRoot().IsArray() );

  const auto &corr_arr = json_correct.GetRoot().AsArray();
  const auto &test_arr = json_testing.GetRoot().AsArray();

  for (const auto &el : corr_arr) {
    const auto &el_dict = el.AsMap();
    if (el_dict.count("total_time") && el_dict.count("request_id")) {
      int req_id = el_dict.at("request_id").AsInt();
      double corr_time = el_dict.at("total_time").AsDouble();
      auto other_time = FindTime(req_id, test_arr);
      BOOST_REQUIRE( other_time.has_value() );
      BOOST_REQUIRE( fabs(other_time.value() - corr_time) < 0.01f );
    }
  }
}