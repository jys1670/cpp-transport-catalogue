#pragma once
#include "input_reader.h"
#include "stat_reader.h"
#include <string>
#include <vector>

template <typename InputStream, typename OutputStream>
struct GenericParserInit {
  GenericParserInit(InputStream &input, OutputStream &output)
      : source{input}, outstream{output}, storage{}, storage_inserter{storage},
        storage_reader{storage, outstream} {};
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

  void ProcessInput();

private:
  GenericParserInit<InputStream, OutputStream> &init;
  std::vector<std::string> raw_queries_;

  using ParseHandler = void (GenericParser::*)(std::string_view);
  const std::unordered_map<QTypes::QTypeId, ParseHandler> command_handlers{
      {QTypes::QTypeId::Stop, &GenericParser::ProcessStopCommand},
      {QTypes::QTypeId::Bus, &GenericParser::ProcessBusCommand},
  };

  template <class Iterator>
  void ParseCommandsBlock(Iterator begin, Iterator end);

  void ProcessStopCommand(std::string_view text);

  void ProcessBusCommand(std::string_view text);

  std::string_view ExtractCommandName(std::string_view text);

  void FlushSavedData() { raw_queries_.clear(); }
};

template <typename InputStream, typename OutputStream>
void GenericParser<InputStream, OutputStream>::ProcessInput() {
  for (std::string queries_amount; std::getline(init.source, queries_amount);) {
    int total_queries = std::stoi(queries_amount);
    std::string query;
    for (int counter{0}; counter < total_queries; ++counter) {
      std::getline(init.source, query);
      raw_queries_.push_back(std::move(query));
    }
    ParseCommandsBlock(raw_queries_.end() - total_queries, raw_queries_.end());
    init.storage_inserter.InsertAllIntoStorage();
    init.storage_reader.PrintAllStats();
    FlushSavedData();
  }
}
template <typename InputStream, typename OutputStream>
template <class Iterator>
void GenericParser<InputStream, OutputStream>::ParseCommandsBlock(
    Iterator begin, Iterator end) {
  const auto &name_to_id = init.known_types.GetNamesToIds();
  for (auto it = begin; it != end; ++it) {
    std::string_view command_name = it->substr(0, it->find_first_of(' '));
    auto handler = command_handlers.at(name_to_id.at(command_name));
    (*this.*handler)(*it);
  }
}

template <typename InputStream, typename OutputStream>
void GenericParser<InputStream, OutputStream>::ProcessStopCommand(
    std::string_view text) {
  auto delim_pos = text.find_first_of(':');
  auto stop_name = ExtractCommandName(text.substr(0, delim_pos));
  auto data = text.substr(delim_pos + 2);
  if (delim_pos == text.npos) { // Route stop_input
    init.storage_reader.ParseCommand(QTypes::QTypeId::Stop, stop_name);
  } else {
    init.storage_inserter.ParseCommand(QTypes::QTypeId::Stop, stop_name, data);
  }
}

template <typename InputStream, typename OutputStream>
void GenericParser<InputStream, OutputStream>::ProcessBusCommand(
    std::string_view text) {
  auto delim_pos = text.find_first_of(':');
  auto bus_name = ExtractCommandName(text.substr(0, delim_pos));
  auto data = text.substr(delim_pos + 2);
  if (delim_pos == text.npos) { // Route stop_input
    init.storage_reader.ParseCommand(QTypes::QTypeId::Bus, bus_name);
  } else { // Route insert
    init.storage_inserter.ParseCommand(QTypes::QTypeId::Bus, bus_name, data);
  }
}

template <typename InputStream, typename OutputStream>
std::string_view GenericParser<InputStream, OutputStream>::ExtractCommandName(
    std::string_view text) {
  auto start = text.find_first_of(' '), end = text.size();
  auto name = text.substr(start, end - start);
  start = name.find_first_not_of(' '), end = name.find_last_not_of(' ');
  name = name.substr(start, end - start + 1);
  return name;
}
