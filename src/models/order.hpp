#pragma once

#include <chrono>
#include <string>

#include <userver/formats/json/value_builder.hpp>

namespace DeliveryService {

struct TOrder {
  std::string id;
  std::string region;
  std::string weight;
  std::string delivery_hours;
  std::string price;
  std::string courier_id;
  std::string completed_time;
};

userver::formats::json::Value Serialize(
    const TOrder& data,
    userver::formats::serialize::To<userver::formats::json::Value>);

}  // namespace DeliveryService