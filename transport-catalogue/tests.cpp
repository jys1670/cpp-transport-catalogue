#include <fstream>
#include <stdexcept>

#include "tests.h"

void CheckTwoNodes(const json::Node &n1, const json::Node &n2,
                   std::string_view comment = "") {
  if (n1.GetValue().index() != n2.GetValue().index()) {
    throw std::runtime_error("Nodes must hold the same type");
  }
  if (n1.IsMap()) {
    auto n1map = n1.AsMap();
    auto n2map = n2.AsMap();
    if (n1map.size() != n1map.size())
      throw std::runtime_error("Maps have different size");
    for (auto it1 = n1map.begin(), it2 = n2map.begin(); it1 != n1map.end();
         ++it1, ++it2) {
      CheckTwoNodes(it1->first, it2->first, "Map keys mismatch");
      if (it1->first != "map") {
        CheckTwoNodes(it1->second, it2->second, "Map values mismatch");
      } else {

//        std::ofstream my_map ("../tests/my_map.svg"),
//        corr_map("../tests/corr_map.svg"); my_map << it1->second.AsString();
//        corr_map << it2->second.AsString();

      }
    }
    return;
  }
  if (n1.IsArray()) {
    auto n1arr = n1.AsArray();
    auto n2arr = n2.AsArray();
    if (n1arr.size() != n1arr.size())
      throw std::runtime_error("Arrays have different size");
    for (auto it1 = n1arr.begin(), it2 = n2arr.begin(); it1 != n1arr.end();
         ++it1, ++it2) {
      CheckTwoNodes(*it1, *it2, "Array elements mismatch");
    }
    return;
  }
  if (n1 != n2) {
    std::cerr << "Expected:" << std::endl;
    PrintNode(n2, {std::cerr});
    std::cerr << std::endl << "But received:" << std::endl;
    PrintNode(n1, {std::cerr});
    std::cerr << std::endl;
    throw std::runtime_error(std::string{comment});
  }
}

void TestBasicInputOutput() {
  std::ifstream input, ostr;
  std::ostringstream output;
  // Awful, but whatever
  input.open("../tests/1.json");
  ostr.open("../tests/1a.json");
  GenericReaderInit tmp{input, output};
  JsonReader test{tmp};
  test.ProcessInput(OutputFormat::Json{});
  std::istringstream input1{output.str()};
  const auto out_correct = json::Load(ostr);
  const auto out_question = json::Load(input1);
  CheckTwoNodes(out_question, out_correct);
}