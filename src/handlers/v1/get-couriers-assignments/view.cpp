#include "view.hpp"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../../models/courier.hpp"
#include "../../../models/order.hpp"
#include <unordered_map>

namespace DeliveryService {

namespace {

class GetCouriersAssignmets final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-get-couriers-assignments";

  GetCouriersAssignmets(const userver::components::ComponentConfig& config,
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
    
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.courier"
        );
 
    
    if (result.IsEmpty()) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }

    std::string assigned_orders = "";
    for(auto courier : result.AsSetOf<TCourier>(
        userver::storages::postgres::kRowTag)) {
        auto orders = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.order "
        "WHERE courier_id=$1",
        courier.id);
      userver::formats::json::ValueBuilder response;
      response["items"].Resize(0);
      for (auto row : orders.AsSetOf<TOrder>(
             userver::storages::postgres::kRowTag)) {
          response["items"].PushBack(row);
      }
      assigned_orders += ToString(userver::formats::json::ValueBuilder{courier}.ExtractValue()) + userver::formats::json::ToString(response.ExtractValue());
    }
    // return assigned_orders key value as string how????

    return "ToString(userver::formats::json::ValueBuilder{courier}.ExtractValue())";
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendGetCouriersAssignmets(userver::components::ComponentList& component_list) {
  component_list.Append<GetCouriersAssignmets>();
}

}  // namespace DeliveryService