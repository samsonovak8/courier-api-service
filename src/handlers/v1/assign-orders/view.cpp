#include "view.hpp"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include <algorithm>
#include <set>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include "../../../models/courier.hpp"
#include "../../../models/order.hpp"

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
    auto uncompleted_orders = GetUncompletedOrders();
    auto couriers = GetSortedCouriers();

    for (const auto& order : uncompleted_orders) {
      auto assigned_courier = FindCourierForOrder(order, couriers);
      if (!assigned_courier.empty()) {
        UpdateOrderWithCourier(order.id, assigned_courier);
      }
    }

    return userver::formats::json::ToString(response.ExtractValue());
  }

 private:
  std::vector<TOrder> GetUncompletedOrders() const {
    auto uncompleted_orders_set = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.order WHERE completed_time = $1", "0");

    return uncompleted_orders_set.AsSetOf<TOrder>(
        userver::storages::postgres::kRowTag);
  }

  std::set<TCourier> GetSortedCouriers() const {
    auto couriers_set = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.courier");

    auto comp = [](const TCourier& a, const TCourier& b) {
      return ConvertToMinutes(a.working_hours) <
             ConvertToMinutes(b.working_hours);
    };

    return std::set<TCourier, decltype(comp)>(
        couriers_set.AsSetOf<TCourier>(userver::storages::postgres::kRowTag)
            .begin(),
        couriers_set.AsSetOf<TCourier>(userver::storages::postgres::kRowTag)
            .end(),
        comp);
  }
  int ConvertToMinutes(const std::string& time) const {
    return ((time[0] - '0') * 10 + (time[1] - '0')) * 60 +
           ((time[3] - '0') * 10 + (time[4] - '0'));
  }
  
  std::string FindCourierForOrder(const TOrder& order,
                                  std::set<TCourier>& couriers) const {
    for (auto& courier : couriers) {

      if (IsCourierEligible(order, courier, order.delivery_hours,
                            courier.working_hours, courier.transport)) {
        couriers.erase(courier);
        courier.working_hours += add;
        couriers.insert(courier);
        return courier.id;
      }
    }

    return "";  // No suitable courier found
  }

  int GetTimeDelay(const std::string& transport) const {
    std::unordered_map<std::string, std::string> time_delay;
    time_delay["пеший"] = 25;
    time_delay["велокурьер"] = 12;
    time_delay["авто"] = 8;
    return time_delay[transport];
  }
  bool CanDeliverOnTime(const std::string& order_delivery_hours,
                        const std::string& courier_working_hours,
                        const std::string& courier_transport) const {
    int delivery_delay = GetTimeDelay(courier.transport);
    int order_interval_begin =
        ConvertToMinutes(order_delivery_hours.substr(0, 5));
    int order_interval_end =
        ConvertToMinutes(order_delivery_hours.substr(6, 11));
    int courier_free_interval_begin = ConvertToMinutes(courier_working_hours);
    return (order_interval_begin <=
            courier_free_interval_begin + delivery_delay) &&
           (courier_free_interval_begin + delivery_delay <= order_interval_end);
  }

  bool IsCourierEligible(const TOrder& order, const TCourier& courier,
                         const std::string& order_delivery_hours,
                         const std::string& courier_working_hours,
                         const std::string& courier_transport) const {
    return std::stoi(order.weight) <= GetMaxWeight(courier.transport) &&
           order.region == courier.region &&
           CanDeliverOnTime(order_delivery_hours, courier_working_hours,
                            delivery_delay);
  }

  int GetMaxWeight(const std::string& transport) const {
    std::unordered_map<std::string, std::string> max_weight;
    max_weight["пеший"] = 10;
    max_weight["велокурьер"] = 20;
    max_weight["авто"] = 40;
    return max_weight[transport];
  }

  void UpdateOrderWithCourier(const std::string& order_id,
                              const std::string& courier_id) const {
    pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "UPDATE delivery_service.order SET courier_id=$1 WHERE id=$2",
        courier_id, order_id);
  }
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendAssignOrders(userver::components::ComponentList& component_list) {
  component_list.Append<AssignOrders>();
}

}  // namespace DeliveryService