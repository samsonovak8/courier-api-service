#pragma once

#include <string>
#include <string_view>

#include <userver/components/component_list.hpp>


namespace DeliveryService {

void AppendGetCouriersAssignmets(userver::components::ComponentList& component_list);

}  // namespace DeliveryService