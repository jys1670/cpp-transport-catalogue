/*!
 * \file json_reader.h
 * \brief Stuff to parse, save and delegate processing of JSON routes & stops
 * input data
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace json {

/*!
 * \brief Responsible for reading JSON input
 *
 * This class is NOT meant to output anything, its main task is input
 * information reading. If input is supposed to modify database, it is handled
 * in place (new routes/stops directly inserted into TransportCatalogue). Other
 * requests are delegated to RequestsHandler (in uniform, JSON independent
 * format), which will process everything once there is nothing left to insert
 * into TransportCatalogue.
 */
class JsonReader {
public:
  /*!
   * Constructor for the class
   * \param[in] inputstream stream from which JSON input will be read
   * \param[in] catalogue database to be modified in place by insertion of
   * new stops and routes
   * \param[in] req_handler requests that are not supposed to modify
   * TransportCatalogue are delegated here
   */
  explicit JsonReader(core::TransportCatalogue &catalogue,
                      core::RequestHandler &req_handler)
      : catalogue_{catalogue}, req_handler_{req_handler} {};

  void ProcessInput(
      const json::Dict &doc_map,
      input_info::OutputFormat format = input_info::OutputFormat::Json);
  void ProcessBaseReqs(const json::Dict &doc_map);
  void EnqueueRenderSettingsUpdate(const json::Dict &doc_map);
  void EnqueueRoutingSettingsUpdate(const json::Dict &doc_map);
  void EnqueueStatReqs(const json::Dict &doc_map);

  //! Handles node that represents single known action defined in RequestTypes
  void ParseSingleCommand(const json::Node &node);

  /*!
   * Inserts all of new routes and stops parsed so far into
   * JsonReader::catalogue_
   */
  void InsertAllIntoCatalogue();

private:
  //! Database to be modified in place with insertion of new stops and routes
  core::TransportCatalogue &catalogue_;
  //! any non-state-changing requests are delegated here
  core::RequestHandler &req_handler_;

  /*!
   * Known types of database state-changing requests.
   * They must be processed in order matching definitions order!
   */
  struct RequestTypes {

    struct StopInsert {
      input_info::Stop *stop_ptr;
    };

    struct StopLinkInsert {
      input_info::StopLink *stoplink_ptr;
    };

    struct BusInsert {
      input_info::Bus *bus_ptr;
    };
  };

  //! Runtime polymorphic type capable of storing all kinds of RequestTypes
  using InputQueue =
      std::variant<RequestTypes::StopInsert, RequestTypes::StopLinkInsert,
                   RequestTypes::BusInsert>;

  /*!
   * Functor used to parse and enqueue TransportCatalogue state-changing
   * requests
   */
  struct JsonInputParse {
  public:
    explicit JsonInputParse(JsonReader &parent) : parent_{parent} {};
    void operator()(std::string_view type, const json::Node &node);

  private:
    using FunctionPtr = void (JsonInputParse::*)(const json::Node &);
    //! Maps between request type string ("Bus", "Stop", etc)
    //! and appropriate member function handler (EnqueueBus, EnqueueStop, etc)
    static std::unordered_map<std::string_view, FunctionPtr> handlers_;
    JsonReader &parent_;
    void EnqueueBus(const json::Node &node);
    void EnqueueStop(const json::Node &node);
  } json_input_parser_{*this};

  /*!
   * Functor used to parse and enqueue TransportCatalogue NON-state-changing
   * requests
   */
  struct JsonPrintParse {
  public:
    using RequestTypes = core::RequestHandler::RequestTypes;
    explicit JsonPrintParse(core::RequestHandler &parent) : parent_{parent} {};
    void operator()(std::string_view type, const json::Node &node);

  private:
    using FunctionPtr = void (JsonPrintParse::*)(const json::Node &);
    static std::unordered_map<std::string_view, FunctionPtr> handlers_;
    core::RequestHandler &parent_;
    void EnqueueBus(const json::Node &node);
    void EnqueueStop(const json::Node &node);
    void EnqueueMapDraw(const json::Node &node);
    void EnqueueRoute(const json::Node &node);
  } json_print_parser_{req_handler_};

  //! Functor used to process elements of JsonReader::input_queue_
  struct CatalogueInserter {
  public:
    explicit CatalogueInserter(JsonReader &parent) : parent_{parent} {};
    void operator()(const RequestTypes::StopInsert &req);
    void operator()(const RequestTypes::StopLinkInsert &req);
    void operator()(const RequestTypes::BusInsert &req);

  private:
    JsonReader &parent_;
  } catalogue_inserter_{*this};

  /*!
   * Queue with all TransportCatalogue state-changing requests,
   * should be sorted to satisfy RequestTypes processing requirements
   */
  std::vector<InputQueue> input_queue_;

  /*!
   * Temporary storage of stops related information.
   * Adding new objects to the end/beginning of deque
   * does not invalidate previously received pointers, which is why this
   * container is used
   */
  std::deque<input_info::Stop> stops_input_queue_;

  //! Temporary storage of stop links related information
  std::deque<input_info::StopLink> stoplinks_input_queue_;

  //! Temporary storage of buses related information
  std::deque<input_info::Bus> buses_input_queue_;

  /*!
   * Clears all temporary JSON data, making JsonReader ready to accept new
   * document
   */
  void Clear();
};

} // namespace json