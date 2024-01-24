#include "courier.hpp"

namespace DeliveryService {

userver::formats::json::Value Serialize(
    const TCourier& courier,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;
  item["id"] = courier.id;
  item["region_id"] = courier.region_id;
  item["transport"] = courier.transport;
  item["max_weight"] = courier.max_weight;
  item["working_time"] = courier.working_time;
  return item.ExtractValue();
}

}  // namespace courierer