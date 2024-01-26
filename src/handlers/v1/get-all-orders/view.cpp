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

class GetAllOrders final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-get-all-orders";

  GetAllOrders(const userver::components::ComponentConfig& config,
              const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {

    std::optional<userver::storages::postgres::ResultSet> result;
    
    result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.order");

    userver::formats::json::ValueBuilder response;
    response["items"].Resize(0);
    for (auto row : result.value().AsSetOf<TOrder>(
             userver::storages::postgres::kRowTag)) {
      response["items"].PushBack(row);
    }

    return userver::formats::json::ToString(response.ExtractValue());
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendGetAllOrders(userver::components::ComponentList& component_list) {
  component_list.Append<GetAllOrders>();
}

}  // namespace DeliveryService