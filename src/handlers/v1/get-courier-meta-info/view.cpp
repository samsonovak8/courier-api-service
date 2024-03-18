#include "view.hpp"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include <unordered_map>
#include <cstdlib>
#include "../../../models/courier.hpp"
#include "../../../models/order.hpp"
#include <chrono>
#include <ctime>
#include <sstream>

namespace DeliveryService {

namespace {

class GetCouriersMetaInfo final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName =
      "handler-v1-get-couriers-assignments";

  GetCouriersMetaInfo(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {
    const auto& id = request.GetPathArg("id");

    auto request_body =
        userver::formats::json::FromString(request.RequestBody());

    auto start_date = request_body["start_date"].As<std::optional<std::string>>();
    auto end_date = request_body["end_date"].As<std::optional<std::string>>();
    if (!dataIsValid(start_date, end_date)) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }

    auto orders = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.order WHERE id=$1 AND completed_date BETWEEN $2 AND $3",
    id, start_date, end_date);

    if (orders.IsEmpty()) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }

    auto courier = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.courier WHERE id=$1",
    id);

    if (courier.IsEmpty()) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }

    int64_t sum = 0;
    int64_t count_orders = 0;
    for (auto row : result.value().AsSetOf<TCourier>(
             userver::storages::postgres::kRowTag)) {
      sum += atoll(row.price);
      ++count_orders;
    }
    int constant_depending_on_courier_type;
    if (courier.type == "пеший") {
        constant_depending_on_courier_type = 2;
    } else if (courier.type == "велокурье") {
        constant_depending_on_courier_type = 3;
    } else {
        constant_depending_on_courier_type = 4;
    }

    std::tm start_tm = string_to_tm(start_date);
    std::tm end_tm = string_to_tm(end_date);
     std::time_t start_time_ = std::mktime(&start_tm);
    std::time_t end_time_ = std::mktime(&end_tm);
    std::chrono::system_clock::time_point start_point = std::chrono::system_clock::from_time_t(start_time_);
    std::chrono::system_clock::time_point end_point = std::chrono::system_clock::from_time_t(end_time_);
    std::chrono::hours duration = std::chrono::duration_cast<std::chrono::hours>(end_point - start_point);
    double rating = (count_orders / static_cast<int>(duration));
    return std::stoll(sum * constant_depending_on_courier_type) + std::stoll(rating * constant_depending_on_courier_type);
  }

 private:
 std::tm string_to_tm(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return tm;
}
 bool dataIsValid(const std::optional<std::string>& start_date,
                   const std::optional<std::string>& end_date) const {
    if (dataIsEmpty(start_date, end_date)) {
      return false;
    }
    std::regex dateRegex("^(\\d{4})-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$");

    
    if (!std::regex_match(start_date, dateRegex) || !std::regex_match(end_date, dateRegex)) {
        return false; 
    }

    return true;
  }
  bool dataIsEmpty(const std::optional<std::string>& start_date,
                   const std::optional<std::string>& end_date) const {
    return !start_date.has_value() || !end_date.has_value();
  }
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendGetCouriersAssignmets(
    userver::components::ComponentList& component_list) {
  component_list.Append<GetCouriersAssignmets>();
}

}  // namespace DeliveryService