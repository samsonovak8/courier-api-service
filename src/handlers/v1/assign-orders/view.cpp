#include "view.hpp"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include <algorithm>
#include <iomanip>
#include <regex>
#include <set>
#include <string>
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
    std::optional<userver::storages::postgres::ResultSet> uncompleted_orders;
    uncompleted_orders = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.order WHERE completed_time = $1", "0");

    auto comp = [&](const TCourier& a, const TCourier& b) {
      return ConvertToMinutes(a.working_hours) <
             ConvertToMinutes(b.working_hours);
    };

    std::set<TCourier, decltype(comp)> couriers(comp);
    auto couriers_set = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM delivery_service.courier");

    for (auto courier :
         couriers_set.AsSetOf<TCourier>(userver::storages::postgres::kRowTag)) {
      couriers.insert(courier);
    }

    std::string assignments = "";
    for (const auto& order : uncompleted_orders.value().AsSetOf<TOrder>(
             userver::storages::postgres::kRowTag)) {
      std::string assigned_courier = "";
      for (auto courier : couriers) {
        if (IsCourierEligible(order, courier, order.delivery_hours,
                              courier.working_hours, courier.transport)) {
          couriers.erase(courier);
          UpdateCouierWorkingHours(courier, order);
          couriers.insert(courier);
          assigned_courier = courier.id;
          break;
        }
      }
      if (assigned_courier != "") {
        assignments +=
            "Order:" +
            userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{order}.ExtractValue()) +
            "was assigned to courier: " +
            userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{assigned_courier}
                    .ExtractValue());

        UpdateOrderWithCourier(order.id, assigned_courier);
      }
    }

    return "userver::formats::json::ToString(response.ExtractValue())";
  }

 private:
  int ConvertToMinutes(const std::string& time) const {
    return ((time[0] - '0') * 10 + (time[1] - '0')) * 60 +
           ((time[3] - '0') * 10 + (time[4] - '0'));
  }

  std::string MinutesToHours(int interval_begin, int interval_end) const {
    int beginHours = interval_begin / 60;
    int beginMins = interval_begin % 60;

    int endHours = interval_end / 60;
    int endMins = interval_end % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << beginHours << ":"
        << std::setw(2) << std::setfill('0') << beginMins << "-" << std::setw(2)
        << std::setfill('0') << endHours << ":" << std::setw(2)
        << std::setfill('0') << endMins;

    return oss.str();
  }

  void UpdateCouierWorkingHours(TCourier& courier, const TOrder& order) const {
    int courier_interval_begin =
        ConvertToMinutes(courier.working_hours.substr(0, 5));
    int courier_interval_end =
        ConvertToMinutes(courier.working_hours.substr(6, 11));
    int order_interval_begin =
        ConvertToMinutes(order.delivery_hours.substr(0, 5));
    int time_delay = GetTimeDelay(courier.transport);
    int new_courier_interval_begin =
        std::max(courier_interval_begin, order_interval_begin) + time_delay;
    courier.working_hours =
        MinutesToHours(new_courier_interval_begin, courier_interval_end);
    return;
  }

  int GetTimeDelay(std::string& transport) const {
    std::unordered_map<std::string, int> time_delay;
    time_delay["пеший"] = 25;
    time_delay["велокурьер"] = 12;
    time_delay["авто"] = 8;
    return time_delay[transport];
  }
  bool CanDeliverOnTime(const std::string& order_delivery_hours,
                        const std::string& courier_working_hours,
                        std::string& courier_transport) const {
    int delivery_delay = GetTimeDelay(courier_transport);
    int order_interval_begin =
        ConvertToMinutes(order_delivery_hours.substr(0, 5));
    int order_interval_end =
        ConvertToMinutes(order_delivery_hours.substr(6, 11));
    // Substr -> InvervalBegin, IntervalEnd
    int courier_free_interval_begin =
        ConvertToMinutes(courier_working_hours.substr(0, 5));
    int courier_free_interval_end =
        ConvertToMinutes(courier_working_hours.substr(6, 11));
    return (order_interval_begin <=
            courier_free_interval_begin + delivery_delay) &&
           (courier_free_interval_begin + delivery_delay <=
            order_interval_end) &&
           (courier_free_interval_begin + delivery_delay <=
            courier_free_interval_end);
  }

  bool IsCourierEligible(const TOrder& order, const TCourier& courier,
                         const std::string& order_delivery_hours,
                         const std::string& courier_working_hours,
                         std::string& courier_transport) const {
    return std::stoi(order.weight) <= GetMaxWeight(courier_transport) &&
           order.region == courier.region &&
           CanDeliverOnTime(order_delivery_hours, courier_working_hours,
                            courier_transport);
  }

  int GetMaxWeight(std::string& transport) const {
    std::unordered_map<std::string, int> max_weight;
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

  struct MinFreeTime {
    bool operator()(const TCourier& a, const TCourier& b){

    };
  };

  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendAssignOrders(userver::components::ComponentList& component_list) {
  component_list.Append<AssignOrders>();
}

}  // namespace DeliveryService