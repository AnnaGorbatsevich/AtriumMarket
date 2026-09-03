
#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/hello.hpp"
#include "handlers/register.hpp"
#include "handlers/login.hpp"
#include "handlers/me.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                               .Append<user_service::HelloHandler>()
                               .Append<user_service::RegisterHandler>()
                               .Append<user_service::LoginHandler>()
                               .Append<user_service::MeHandler>()
                               .Append<userver::components::Postgres>("postgres-db")
                               .Append<userver::components::TestsuiteSupport>()
                               .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
