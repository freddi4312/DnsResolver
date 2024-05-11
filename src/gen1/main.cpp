#include <iostream>
#include "Resolver.h"
#include "Receiver.h"
#include "Cache.h"

int main()
{
  sockpp::socket_initializer SocketInit;

  std::vector<std::pair<std::string, std::string>> const root_servers =
  {
    { "a.root-servers.net", "198.41.0.4" },
    { "b.root-servers.net", "199.9.14.201" },
    { "c.root-servers.net", "192.33.4.12" },
    { "d.root-servers.net", "199.7.91.13" },
    { "e.root-servers.net", "192.203.230.10" },
    { "f.root-servers.net", "192.5.5.241" },
    { "g.root-servers.net", "192.112.36.4" },
    { "h.root-servers.net", "198.97.190.53" },
    { "i.root-servers.net", "192.36.148.17" },
    { "j.root-servers.net", "192.58.128.30" },
    { "k.root-servers.net", "193.0.14.129" },
    { "l.root-servers.net", "199.7.83.42" },
    { "m.root-servers.net", "202.12.27.33" }
  };

  Cache::getInstance().addRootServers(root_servers);

  /*
  Resolver resolver("music.yandex.ru", Tins::DNS::QueryType::A, Tins::DNS::QueryClass::INTERNET);

  std::string result;
  if (resolver.resolve() == 0)
  {
    result = resolver.getAddress();
  }
  else
  {
    result = "error";
  }

  std::cout << result << std::endl;
  */

  UdpReceiver receiver(dnsPort);
  //TcpReceiver receiver(30000);

  return receiver.run();
}
