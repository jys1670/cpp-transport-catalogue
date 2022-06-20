#include <iomanip>
#include <stdexcept>
#include <string>
#include <string_view>

#include "input_reader.h"
#include "stat_reader.h"
#include "tests.h"

template <typename InputStream, typename OutputStream>
struct GenericParserInit {
  GenericParserInit(InputStream &input, OutputStream &output)
      : source{input}, outstream{output}, storage{},
        storage_inserter{storage, known_types}, storage_reader{storage,
                                                               outstream,
                                                               known_types} {};
  InputStream &source;
  OutputStream &outstream;
  TransportCatalogue storage;
  QTypes known_types{
      {"Stop", QTypes::QTypeId::Stop},
      {"Bus", QTypes::QTypeId::Bus},
  };
  InputReader storage_inserter;
  StatReader<OutputStream> storage_reader;
};

template <typename InputStream, typename OutputStream> class GenericParser {
public:
  GenericParser(GenericParserInit<InputStream, OutputStream> &input)
      : init{input} {};

  void ProcessInput() {
    for (std::string queries_amount;
         std::getline(init.source, queries_amount);) {
      int total_queries = std::stoi(queries_amount);
      std::string query;
      for (int counter{0}; counter < total_queries; ++counter) {
        std::getline(init.source, query);
        raw_queries_.push_back(std::move(query));
      }
      ParseCommandsBlock(raw_queries_.end() - total_queries,
                         raw_queries_.end());
      init.storage_inserter.InsertAllIntoStorage();
      init.storage_reader.PrintAllStats();
      Clear();
    }
  }

private:
  GenericParserInit<InputStream, OutputStream> &init;
  std::vector<std::string> raw_queries_;

  template <class Iterator>
  void ParseCommandsBlock(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      std::string_view::size_type delim_pos = it->find_first_of(':');
      if (delim_pos != std::string_view::npos) {
        init.storage_inserter.ParseCommand(*it);
      } else {
        init.storage_reader.ParseCommand(*it);
      }
    }
  }

  void Clear() { raw_queries_.clear(); }
};

void TestBasicInputOutput() {
  std::string input{
      "13\n"
      "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Maru   shkino\n"
      "Stop Maru   shkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to "
      "Maru   shkino\n"
      "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo "
      "Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
      "Bus 750: Tolstopaltsevo - Maru   shkino - Maru   shkino - Rasskazovka\n"
      "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Maru   shkino\n"
      "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya "
      "ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
      "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
      "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, "
      "900m to Biryulyovo Tovarnaya\n"
      "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo "
      "Passazhirskaya\n"
      "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to "
      "Biryulyovo Zapadnoye\n"
      "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > "
      "Biryulyovo Zapadnoye\n"
      "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
      "Stop Prazhskaya: 55.611678, 37.603831\n"
      "9\n"
      "Bus 256\n"
      "Stop Samara\n"
      "Bus 750\n"
      "Bus 751\n"
      "Stop Maru   shkino\n"
      "Stop Prazhskaya\n"
      "Stop Biryulyovo Zapadnoye\n"
      "Bus 828\n"
      "Stop Biryuly  ovo Zapadnoye\n"},
      output = {"Bus 256: 6 stops on route, 5 unique stops, 5950 route length, "
                "1.36124 curvature\n"
                "Stop Samara: not found\n"
                "Bus 750: 7 stops on route, 3 unique stops, 27400 route "
                "length, 1.30853 curvature\n"
                "Bus 751: not found\n"
                "Stop Maru   shkino: buses 750\n"
                "Stop Prazhskaya: no buses\n"
                "Stop Biryulyovo Zapadnoye: buses 256 828\n"
                "Bus 828: 4 stops on route, 3 unique stops, 15500 route "
                "length, 1.95908 curvature\n"
                "Stop Biryuly  ovo Zapadnoye: not found"};

  std::istringstream input_stream{input};
  std::ostringstream output_stream{};
  output_stream << std::setprecision(6);
  GenericParserInit<std::istream, std::ostream> parser_input{input_stream,
                                                             output_stream};
  GenericParser<std::istream, std::ostream> test_parser{parser_input};
  test_parser.ProcessInput();

  std::istringstream original{output}, processed{output_stream.str()};
  std::string correct_line, actual_line;
  while (std::getline(original, correct_line)) {
    std::getline(processed, actual_line);
    if (correct_line != actual_line) {
      std::cerr << "Correct line:\n"
                << correct_line << "\nActual line:\n"
                << actual_line << std::endl;
      throw std::runtime_error("Test failed: incorrect output detected");
    }
  }
}