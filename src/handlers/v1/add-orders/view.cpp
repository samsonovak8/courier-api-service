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

class AddOrders final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-add-orders";

  AddOrders(const userver::components::ComponentConfig& config,
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

    auto region = request_body["region"].As<std::optional<std::string>>();
    auto weight= request_body["weight"].As<std::optional<std::string>>();
    auto delivery_hours = request_body["delivery_hours"].As<std::optional<std::string>>();
    auto price = request_body["price"].As<std::optional<std::string>>();
    

    if (!region.has_value() || !price.has_value() || !weight.has_value() || !delivery_hours.has_value()) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO delivery_service.order(region, weight, delivery_hours, price, courier_id, completed_time) VALUES($1, $2, $3, $4, $5, $6) "
        "ON CONFLICT DO NOTHING "
        "RETURNING *",
        region.value(), weight.value(), delivery_hours.value(), price.value(), "1", "0");

    if (result.IsEmpty()) {
      throw std::runtime_error("Orders insertion conflict");
    }

    auto order =
        result.AsSingleRow<TOrder>(userver::storages::postgres::kRowTag);
    return ToString(
        userver::formats::json::ValueBuilder{order}.ExtractValue());
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendAddOrders(userver::components::ComponentList& component_list) {
  component_list.Append<AddOrders>();
}

}  // namespace DeliveryService