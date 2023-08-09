#include <iostream>
#include <vector>
#include <map>
#include <sockpp/connector.h>
#include <sockpp/inet_address.h>
#define TINS_STATIC
#include <tins/dns.h>


class Resolver
{
public:
  Resolver(std::vector<std::string> const &RootIP);
  std::string resolve(std::string const &DomainName);

private:
  std::vector<std::string> splitDomainName(std::string const &DomainName) const;
  Tins::DNS getPduDnsNsRequest(std::string const &ShortName) const;
  Tins::DNS getPduDnsARequest(std::string const &ShortName) const;
  void sendPduDns(Tins::DNS &PduDns);
  Tins::DNS readPduDns();
  std::vector<sockpp::inet_address> getNsAddrs(Tins::DNS const &PduDns) const;


  sockpp::connector _TcpConnector;
  std::vector<sockpp::inet_address> _RootAddr;
  std::map<std::string, uint32_t> _CacheTypeA;
};