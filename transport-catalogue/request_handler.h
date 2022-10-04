/*!
 * \file request_handler.h
 * \brief Things to query and print TransportCatalogue information
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
#include "transport_catalogue.h"
#include "transport_router.h"

namespace core {
/*!
 * \brief Responsible for querying database (TransportCatalogue) and
 * outputting information in various formats.
 *
 * This class is NOT meant to modify TransportCatalogue. It is used as passive
 * database query and print tool capable of working with multiple
 * output formats.
 */
class RequestHandler {
public:
  /*!
   * Known types of database query (NON-state-changing) requests.
   * Processing order does not matter as of now.
   */
  struct RequestTypes {

    struct PrintBusStats {
      int id;
      std::string_view bus_name;
    };

    struct PrintStopStats {
      int id;
      std::string_view stop_name;
    };

    struct UpdateMapRenderSettings {
      const json::Node *settings;
    };

    struct PrintMap {
      int id;
    };

    struct UpdateRoutingSettings {
      const json::Node *settings;
    };

    struct Route {
      int id;
      std::string_view from;
      std::string_view to;
    };
  };

  //! Runtime polymorphic type capable of storing all kinds of RequestTypes
  using ReqsQueue =
      std::variant<RequestTypes::PrintBusStats, RequestTypes::PrintStopStats,
                   RequestTypes::UpdateMapRenderSettings,
                   RequestTypes::PrintMap, RequestTypes::UpdateRoutingSettings,
                   RequestTypes::Route>;

  /*!
   * Constructor for the class
   * \param[in] output stream to which information will be output
   * \param[in] catalogue database which will be queried
   * \param[in] renderer will be used for map visualize requests when needed
   */
  RequestHandler(std::ostream &output, core::TransportCatalogue &catalogue,
                 graphics::MapRenderer &renderer, core::TransportRouter &router)
      : outstream_{output},
        catalogue_{catalogue}, renderer_{renderer}, trouter_{router} {};

  //! Inserts new element into RequestHandler::reqs_queue_
  void InsertIntoQueue(ReqsQueue &&elem);

  /*!
   * Extracts ALL routes and THEIR stops (not all stops,
   * just those used in routes, with no repetitions) from
   * RequestHandler::catalogue_
   */
  data::RoutesData GetCatalogueData() const;

  //! Processes each element from RequestHandler::reqs_queue_
  void ProcessAllRequests(
      input_info::OutputFormat format = input_info::OutputFormat::Json);

private:
  //! Stream to which information will be output
  std::ostream &outstream_;
  //! Database that processes requests for information
  core::TransportCatalogue &catalogue_;
  //! Renderer to handle map visualize requests when needed
  graphics::MapRenderer &renderer_;
  //! Transport router to handle fastest path requests when needed
  core::TransportRouter &trouter_;

  //! Functor used to respond to RequestHandler::reqs_queue_ requests in JSON
  //! format
  struct JsonPrint {
  public:
    explicit JsonPrint(RequestHandler &parent, json::Array &arr)
        : parent_{parent}, arr_{arr} {};
    void operator()(const RequestTypes::PrintBusStats &req);
    void operator()(const RequestTypes::PrintStopStats &req);
    void operator()(const RequestTypes::UpdateMapRenderSettings &req);
    void operator()(const RequestTypes::PrintMap &req);
    void operator()(const RequestTypes::UpdateRoutingSettings &req);
    void operator()(const RequestTypes::Route &req);

  private:
    RequestHandler &parent_;
    json::Array &arr_;
  };

  //! Queue with all TransportCatalogue NON-state-changing requests
  std::vector<ReqsQueue> reqs_queue_;

  //! Clears RequestHandler::reqs_queue_
  void Clear();
};
} // namespace core