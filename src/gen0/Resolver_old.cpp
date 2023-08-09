#include <sstream>
#include <map>
#include "Resolver.h"


Resolver::Resolver(std::vector<std::string> const &RootIP) :
  _TcpConnector(),
  _RootAddr(),
  _CacheTypeA()
{
  _RootAddr.reserve(RootIP.size());

  for (std::string const &str : RootIP)
    _RootAddr.push_back(sockpp::inet_address(str, 53));
}


std::vector<std::string> Resolver::splitDomainName(std::string const &DomainName) const
{
  std::vector<std::string> Labels{};
  std::istringstream StrIn(DomainName);
  std::string Label;

  while (std::getline(StrIn, Label, '.'))
    Labels.push_back(Label);

  std::vector<std::string> ShortNames(Labels.size());
  ShortNames[0] = Labels.back();
  for (int i = 1; i < Labels.size(); i++)
    ShortNames[i] = Labels[Labels.size() - 1 - i] + "." + ShortNames[i - 1];

  return ShortNames;
}


Tins::DNS Resolver::getPduDnsNsRequest(std::string const &ShortName) const
{
  Tins::DNS PduDns{};
  PduDns.id(30);
  PduDns.type(Tins::DNS::QRType::QUERY);
  PduDns.opcode(0);
  Tins::DNS::Query DnsQuery(ShortName, Tins::DNS::QueryType::NS, Tins::DNS::QueryClass::IN);
  PduDns.add_query(DnsQuery);

  return PduDns;
}


Tins::DNS Resolver::getPduDnsARequest(std::string const &DomainName) const
{
  Tins::DNS PduDns{};
  PduDns.id(30);
  PduDns.type(Tins::DNS::QRType::QUERY);
  PduDns.opcode(0);
  Tins::DNS::Query DnsQuery(DomainName, Tins::DNS::QueryType::A, Tins::DNS::QueryClass::IN);
  PduDns.add_query(DnsQuery);

  return PduDns;
}


void Resolver::sendPduDns(Tins::DNS &PduDns)
{
  uint16_t MessageSize = htons(PduDns.size());

  if (_TcpConnector.write_n(&MessageSize, sizeof(uint16_t)) != sizeof(uint16_t))
    throw std::exception("PduDns size sending failure");

  if (_TcpConnector.write_n(PduDns.serialize().data(), PduDns.size()) != PduDns.size())
    throw std::exception("PduDns sending failure");
}


Tins::DNS Resolver::readPduDns()
{
  uint16_t PduSize = 0;

  if (_TcpConnector.read_n(&PduSize, sizeof(PduSize)) != sizeof(PduSize))
    throw std::exception("PduDns size reading failure");

  PduSize = ntohs(PduSize);

  std::vector<uint8_t> PduBuffer(PduSize);
  if (_TcpConnector.read_n(PduBuffer.data(), PduBuffer.size()) != PduBuffer.size())
    throw std::exception("PduDns reading failure");

  return Tins::DNS(PduBuffer.data(), PduBuffer.size());
}


std::vector<sockpp::inet_address> Resolver::getNsAddrs(Tins::DNS const &PduDns) const
{
  std::vector<sockpp::inet_address> NsAddrs{};
  NsAddrs.reserve(PduDns.authority_count());

  std::map<std::string, std::string> NameToIp{};

  for (auto const &RR : PduDns.additional())
    if (RR.query_class() == Tins::DNS::QueryClass::IN && RR.query_type() == Tins::DNS::QueryType::A)
      NameToIp[RR.dname()] = RR.data();

  for (auto const &RR : PduDns.authority())
    if (NameToIp.find(RR.data()) != NameToIp.end())
      NsAddrs.push_back(sockpp::inet_address(NameToIp[RR.data()], 53));

  return NsAddrs;
}


std::string Resolver::resolve(std::string const &DomainName)
{
  std::vector<std::string> ShortNames = splitDomainName(DomainName);
  std::vector<sockpp::inet_address> NsAddr(_RootAddr);
  size_t const LevelsCount = ShortNames.size();
  size_t Level = 0;
  size_t NsCount = NsAddr.size();
  size_t NsNumber = 0;

  Tins::DNS PduDns = getPduDnsNsRequest(ShortNames[Level]);

  while (Level < LevelsCount)
  {
    try
    {
      if (!_TcpConnector.connect(NsAddr[NsNumber]))
        throw std::exception(std::string("Connection failure: " + NsAddr[NsNumber].to_string()).c_str());

      sendPduDns(PduDns);
      PduDns = readPduDns();
      NsAddr = getNsAddrs(PduDns);
      if (NsAddr.size() == 0)
        throw std::exception("No answers in PDU");


      Level++;
      NsCount = NsAddr.size();
      NsNumber = 0;
      if (Level < LevelsCount)
        PduDns = getPduDnsNsRequest(ShortNames[Level]);
    }
    catch (Tins::malformed_packet &)
    {
      std::cout << "Received malformed packet";
      NsNumber++;
    }
    catch (std::exception & e)
    {
      std::cout << e.what() << "\n";
      std::cout << "Error " << _TcpConnector.last_error();
      std::cout << ": " << _TcpConnector.last_error_str() << std::endl;
      NsNumber++;
    }
    
    if (NsNumber == NsCount)
      return std::string{};
  }

  while (NsNumber < NsCount)
  {
    try
    {
      PduDns = getPduDnsARequest(ShortNames.back());

      if (!_TcpConnector.connect(NsAddr[NsNumber]))
        throw std::exception(std::string("Connection failure: " + NsAddr[NsNumber].to_string()).c_str());

      sendPduDns(PduDns);
      PduDns = readPduDns();
      if (PduDns.answers_count() == 0)
        throw std::exception("No answers in PDU");
      else
        return PduDns.answers()[0].data();
    }
    catch (Tins::malformed_packet &)
    {
      std::cout << "Received malformed packet";
      NsNumber++;
    }
    catch (std::exception & e)
    {
      std::cout << e.what() << "\n";
      std::cout << "Error " << _TcpConnector.last_error();
      std::cout << ": " << _TcpConnector.last_error_str() << std::endl;
      NsNumber++;
    }
  }

  return std::string{};
}