#include "view.hpp"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../../models/courier.hpp"

namespace DeliveryService {

namespace {

class AddCouriers final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-add-couriers";

  AddCouriers(const userver::components::ComponentConfig& config,
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
    auto transport = request_body["transport"].As<std::optional<std::string>>();
    auto working_hours = request_body["working_hours"].As<std::optional<std::string>>();
    
    if (!region.has_value() || !transport.has_value() || !working_hours.has_value()) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }
  
    // auto result = pg_cluster_->Execute(
    //   userver::storages::postgres::ClusterHostType::kMaster,
    //   "SELECT * FROM delivery_service.courier");
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO delivery_service.courier(region, transport, working_hours) VALUES($1, $2, $3) "
        "ON CONFLICT DO NOTHING "
        "RETURNING *",
        region.value(), transport.value(), working_hours.value());

    if (result.IsEmpty()) {
      throw std::runtime_error("Couriers insertion conflict");
    }

    auto courier =
        result.AsSingleRow<TCourier>(userver::storages::postgres::kRowTag);
    return ToString(
        userver::formats::json::ValueBuilder{courier}.ExtractValue());
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendAddCouriers(userver::components::ComponentList& component_list) {
  component_list.Append<AddCouriers>();
}

}  // namespace DeliveryService