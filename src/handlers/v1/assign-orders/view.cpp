#include "view.hpp"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../../models/order.hpp"
#include "../../../models/courier.hpp"
#include <typeinfo>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_map>

namespace DeliveryService {

namespace {

class AssignOrders final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-assign-orders";

  AssignOrders(const userver::components::ComponentConfig& config,
              const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {
      
    auto uncompleted_orders_set = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.order "
        "WHERE completed_time = $1",
        "0");

    std::vector<TOrder> uncompleted_orders;
    userver::formats::json::ValueBuilder response;
    response["items"].Resize(0);
    for (auto row : uncompleted_orders_set.AsSetOf<TOrder>(
             userver::storages::postgres::kRowTag)) {
      //response["items"].PushBack(row);
      uncompleted_orders.push_back(row);
    }
    std::sort(uncompleted_orders.begin(), uncompleted_orders.end(), [&](TOrder& a, TOrder& b) {
      int time1 = ((a.delivery_hours[0] - '0') * 10 + (a.delivery_hours[1] - '0')) * 60 + (a.delivery_hours[3] - '0') * 10 + (a.delivery_hours[4] - '0');
      int time2 = ((b.delivery_hours[0] - '0') * 10 + (b.delivery_hours[1] - '0')) * 60 + (b.delivery_hours[3] - '0') * 10 + (b.delivery_hours[4] - '0');
      return time1 < time2;
    });
    
    auto couriers_set = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.courier"
        );
    
    auto comp = [](const TCourier& a, const TCourier& b) {
      int time1 = ((a.working_hours[0] - '0') * 10 + (a.working_hours[1] - '0')) * 60 + (a.working_hours[3] - '0') * 10 + (a.working_hours[4] - '0');
      int time2 = ((b.working_hours[0] - '0') * 10 + (b.working_hours[1] - '0')) * 60 + (b.working_hours[3] - '0') * 10 + (b.working_hours[4] - '0');
      return time1 < time2;
    };
    std::set<TCourier, decltype(comp)> couriers(comp);
    for (auto row : couriers_set.AsSetOf<TCourier>(
            userver::storages::postgres::kRowTag))  {
        couriers.insert(row);
      }
      
    for (auto order : uncompleted_orders) {
      bool courier_found = false;
      std::string assigned_courier = "";
      for (auto courier : couriers) {
        int time1 = ((order.delivery_hours[0] - '0') * 10 + (order.delivery_hours[1] - '0')) * 60 + ((order.delivery_hours[3] - '0') * 10 + (order.delivery_hours[4] - '0'));
        int time2 = ((order.delivery_hours[6] - '0') * 10 + (order.delivery_hours[7] - '0')) * 60 + ((order.delivery_hours[9] - '0') * 10 + (order.delivery_hours[10] - '0'));
        int time3 = ((courier.working_hours[0] - '0') * 10 + (courier.working_hours[1] - '0')) * 60 + ((courier.working_hours[3] - '0') * 10 + (courier.working_hours[4] - '0'));
        int add = 0;
        if (courier.transport == "пеший") {
          add = 25;
          if (std::stoi(order.weight) <= 10 && order.region == courier.region && time3 + add >= time1 && time3 + add <= time2) {
            courier_found = true;
          }
        } else if (courier.transport == "велокурьер") {
          add = 12;
          if (std::stoi(order.weight) <= 20 && time3 + add >= time1 && time3 + add <= time2) {
            courier_found = true;
          }
        } else {
          add = 8;
          if (std::stoi(order.weight) <= 40 && time3 + add >= time1 && time3 + add <= time2) {
            courier_found = true;
          }
        }
        if (courier_found) {
          assigned_courier = courier.id;
          couriers.erase(courier);
          // courier.working_hours += add;
          couriers.insert(courier);
          break;
        }
      }
     
      auto couriers_set = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "UPDATE delivery_service.order "
        "SET courier_id=$1 "
        "WHERE id=$2",
        assigned_courier, order.id);
    }
    // return ???
    
    return "userver::formats::json::ToString(response.ExtractValue())";
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendAssignOrders(userver::components::ComponentList& component_list) {
  component_list.Append<AssignOrders>();
}

}  // namespace DeliveryService