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
  explicit JsonReader(std::istream &inputstream, TransportCatalogue &catalogue,
                      RequestHandler &req_handler)
      : inputstream_{inputstream}, catalogue_{catalogue}, req_handler_{
                                                              req_handler} {};

  /*!
   * Attempts to read entire JSON document from JsonReader::inputstream_,
   * modifies JsonReader::catalogue_ if needed, then calls
   * JsonReader::req_handler_ to output whatever was asked to print.
   * \param[in] OutputFormat::Json specifies RequestHandler output format
   */
  void ProcessInput(OutputFormat::Json);

  //! Handles node that represents single known action defined in RequestTypes
  void ParseSingleCommand(const json::Node &node);

  /*!
   * Inserts all of new routes and stops parsed so far into
   * JsonReader::catalogue_
   */
  void InsertAllIntoCatalogue();

private:
  //! Stream from which JSON input will be read
  std::istream &inputstream_;
  //! Database to be modified in place by insertion of new stops and routes
  TransportCatalogue &catalogue_;
  //! any non-state-changing requests are delegated here
  RequestHandler &req_handler_;

  /*!
   * Known types of database state-changing requests.
   * They must be processed in order matching definitions order!
   */
  struct RequestTypes {

    struct StopInsert {
      InputInfo::Stop *stop_ptr;
    };

    struct StopLinkInsert {
      InputInfo::StopLink *stoplink_ptr;
    };

    struct BusInsert {
      InputInfo::Bus *bus_ptr;
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
    using RequestTypes = RequestHandler::RequestTypes;
    explicit JsonPrintParse(RequestHandler &parent) : parent_{parent} {};
    void operator()(std::string_view type, const json::Node &node);

  private:
    using FunctionPtr = void (JsonPrintParse::*)(const json::Node &);
    static std::unordered_map<std::string_view, FunctionPtr> handlers_;
    RequestHandler &parent_;
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
  std::deque<InputInfo::Stop> stops_input_queue_;

  //! Temporary storage of stop links related information
  std::deque<InputInfo::StopLink> stoplinks_input_queue_;

  //! Temporary storage of buses related information
  std::deque<InputInfo::Bus> buses_input_queue_;

  /*!
   * Clears all temporary JSON data, making JsonReader ready to accept new
   * document
   */
  void Clear();
};