#include "view.hpp"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../../models/order.hpp"

namespace DeliveryService {

namespace {

class CompleteOrders final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-complete-orders";

  CompleteOrders(const userver::components::ComponentConfig& config,
              const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {
      
     auto request_body =
        userver::formats::json::FromString(request.RequestBody());

    auto courier_id = request_body["courier_id"].As<std::optional<std::string>>();
    auto order_id = request_body["order_id"].As<std::optional<std::string>>();
    auto completed_time = request_body["completed_time"].As<std::optional<std::string>>();

    if (!dataIsValid(courier_id, order_id, completed_time)) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }
    
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.order "
        "WHERE id = $1",
        order_id.value());
      
    
    if (result.IsEmpty()) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }

    auto order =
        result.AsSingleRow<TOrder>(userver::storages::postgres::kRowTag);
    
    if (order.courier_id != courier_id.value() || order.completed_time != "0") {
        auto& response = request.GetHttpResponse();
        response.SetStatus(userver::server::http::HttpStatus::kNotFound);
        return {};
    }

    result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "UPDATE delivery_service.order "
        "SET completed_time = $1 "
        "WHERE id = $2 "
        "RETURNING *",
        completed_time.value(), order_id.value());
      
    if (result.IsEmpty()) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }
    auto updated_order =
        result.AsSingleRow<TOrder>(userver::storages::postgres::kRowTag);
    
    return ToString(userver::formats::json::ValueBuilder{updated_order}.ExtractValue());
  }

 private:
 bool dataIsValid(const std::optional<std::string>& courier_id,
                   const std::optional<std::string>& order_id,
                   const std::optional<std::string>& completed_time) const {
    if (dataIsEmpty(courier_id, order_id, completed_time)) {
      return false;
    }

    std::regex validTime(R"(([01][0-9]|2[0-3]):[0-5][0-9])");
    if (completed_time && !std::regex_match(*completed_time, validTime)) {
      return false;
    }

    return true;
  }
  bool dataIsEmpty(const std::optional<std::string>& courier_id,
                   const std::optional<std::string>& order_id,
                   const std::optional<std::string>& completed_time) const {
    return !courier_id.has_value() || !order_id.has_value() ||
           !completed_time.has_value();
  }
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendCompleteOrders(userver::components::ComponentList& component_list) {
  component_list.Append<CompleteOrders>();
}

}  // namespace DeliveryService