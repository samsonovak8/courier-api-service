#include "courier.hpp"

namespace DeliveryService {

userver::formats::json::Value Serialize(
    const TCourier& courier,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;
  item["id"] = courier.id;
  item["region"] = courier.region;
  item["transport"] = courier.transport;
  item["max_weight"] = courier.max_weight;
  item["working_hours"] = courier.working_hours;
  return item.ExtractValue();
}

}  // namespace courierer