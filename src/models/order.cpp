#include "order.hpp"

namespace DeliveryService {

userver::formats::json::Value Serialize(
    const TOrder& order,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;
  item["id"] = order.id;
  item["region"] = order.region;
  item["weight"] = order.weight;
  item["delivery_hours"] = order.delivery_hours;
  item["price"] = order.price;
  item["courier_id"] = order.courier_id;
  item["completed_time"] = order.completed_time;
  return item.ExtractValue();
}

}  // namespace courierer