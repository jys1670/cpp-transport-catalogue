#include <iomanip>
#include <iostream>
#include <vector>

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
  explicit GenericParser(GenericParserInit<InputStream, OutputStream> &input)
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
      std::string_view::size_type colon_pos = it->find_first_of(':');
      if (colon_pos != std::string_view::npos) {
        init.storage_inserter.ParseCommand(*it);
      } else {
        init.storage_reader.ParseCommand(*it);
      }
    }
  }

  void Clear() { raw_queries_.clear(); }
};

int main() {
  TestBasicInputOutput();
  /* Аргументировал в slack */
  GenericParserInit<std::istream, std::ostream> parser_input{std::cin,
                                                             std::cout};
  GenericParser<std::istream, std::ostream> parser{parser_input};
  parser.ProcessInput();
}