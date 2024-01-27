#pragma once

#include <chrono>
#include <string>

#include <userver/formats/json/value_builder.hpp>

namespace DeliveryService {

struct TCourier {
  std::string id;
  std::string region;
  std::string transport;
  std::string working_hours;
};

userver::formats::json::Value Serialize(
    const TCourier& data,
    userver::formats::serialize::To<userver::formats::json::Value>);

}  // namespace DeliveryService