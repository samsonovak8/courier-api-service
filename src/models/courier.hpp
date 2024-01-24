#pragma once

#include <chrono>
#include <string>

#include <userver/formats/json/value_builder.hpp>

namespace DeliveryService {

struct TCourier {
  std::string id;
  std::string region_id;
  std::string transport;
  std::string max_weight;
  std::string working_time;
};

userver::formats::json::Value Serialize(
    const TCourier& data,
    userver::formats::serialize::To<userver::formats::json::Value>);

}  // namespace DeliveryService