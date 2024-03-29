#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>


#include "handlers/v1/get-courier/view.hpp"
#include "handlers/v1/get-all-couriers/view.hpp"
#include "handlers/v1/add-couriers/view.hpp"

#include "handlers/v1/get-all-orders/view.hpp"
#include "handlers/v1/get-order/view.hpp"
#include "handlers/v1/add-orders/view.hpp"
#include "handlers/v1/complete-orders/view.hpp"
#include "handlers/v1/assign-orders/view.hpp"
#include "handlers/v1/get-couriers-assignments/view.hpp"

#include "hello.hpp"

int main(int argc, char* argv[]) {
  auto component_list = userver::components::MinimalServerComponentList()
                            .Append<userver::server::handlers::Ping>()
                            .Append<userver::components::TestsuiteSupport>()
                            .Append<userver::components::HttpClient>()
                            .Append<userver::server::handlers::TestsControl>()
                            .Append<userver::components::Postgres>("postgres-db-1")
                            .Append<userver::clients::dns::Component>();

  enrollment_template::AppendHello(component_list);

  DeliveryService::AppendGetCourier(component_list);
  DeliveryService::AppendGetAllCouriers(component_list);
  DeliveryService::AppendAddCouriers(component_list);

  DeliveryService::AppendGetAllOrders(component_list);
  DeliveryService::AppendGetOrder(component_list);
  DeliveryService::AppendAddOrders(component_list);

  DeliveryService::AppendCompleteOrders(component_list);
  DeliveryService::AppendAssignOrders(component_list);
  DeliveryService::AppendGetCouriersAssignmets(component_list);
  return userver::utils::DaemonMain(argc, argv, component_list);
}
