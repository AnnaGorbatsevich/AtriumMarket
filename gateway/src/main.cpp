
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/register_proxy.hpp"
#include "handlers/hello.hpp"
#include "handlers/login_proxy.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                               .Append<gateway::RegisterProxyHandler>()
                               .Append<gateway::HelloHandler>()
                               .Append<gateway::LoginProxyHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
