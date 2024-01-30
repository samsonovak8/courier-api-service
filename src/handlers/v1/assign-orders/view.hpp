#pragma once

#include <string>
#include <string_view>

#include <userver/components/component_list.hpp>


namespace DeliveryService {

void AppendAssignOrders(userver::components::ComponentList& component_list);

}  // namespace DeliveryService