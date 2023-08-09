#include <sstream>
#include <list>
#include <memory>

#include "Resolver.h"
#include "LocalCache.h"
#define TINS_STATIC
#include <tins/dns.h>
#include <sockpp/connector.h>
#include <sockpp/inet_address.h>
#include <magic_enum.hpp>


struct DomainName
{
public:
  explicit DomainName(std::string const & name)
  {
    std::istringstream strIn(name);
	  std::string label{};
  
	  while (std::getline(strIn, label, '.'))
	    _labels.emplace_front(label);
  }

  size_t GetLabelsCount() const
  {
	  return _labels.size();
  }

  std::string GetName(size_t level) const
  {
	  std::string name{};
	  auto It = _labels.begin();

    while (level-- > 0)
	  {
      name = *It + name;
	    if (level != 0)
	      name = '.' + name;

	    It++;
	  }

	  return name;
  }

  std::string GetFullName() const
  {
	  return GetName(_labels.size());
  }

private:
  std::list<std::string> _labels;
};

size_t GetStartLevel(LocalCache & cache, DomainName const & dName, std::string & ip)
{
  size_t level = dName.GetLabelsCount();
  bool isFound = false;

  ++level;
  while (!isFound && level > 0)
  {
    --level;

    if (cache.hasNS(dName.GetName(level)))
    {
      for (auto const &ns : cache.getNS(dName.GetName(level)))
        if (cache.hasA(ns))
        {
          isFound = true;
          ip = cache.getA(ns);
          break;
        }
    }
  }

  return level + 1;
}


Tins::DNS makePduDnsQuerry(std::string const & name, Tins::DNS::QueryType queryType)
{
  Tins::DNS::query Query
  (
    name,
	  queryType,
	  Tins::DNS::QueryClass::IN
  );

  Tins::DNS PduDns{};

  PduDns.id(1);
  PduDns.type(Tins::DNS::QRType::QUERY);
  PduDns.opcode(0);
  //PduDns.recursion_desired(1);
  PduDns.add_query(Query);

  return PduDns;
}


ssize_t SendPduDns(std::shared_ptr<sockpp::connector> Connector, Tins::DNS & PduDns)
{
  ssize_t Result = -1;
  uint16_t PduDnsSize = htons(static_cast<uint16_t>(PduDns.size()));

  Result = Connector->write_n(&PduDnsSize, sizeof(PduDnsSize));
  if (Result == -1)
    return Result;

  Result = Connector->write_n(PduDns.serialize().data(), PduDns.size());

  return Result;
}


ssize_t ReadPduDns(std::shared_ptr<sockpp::connector> Connector, Tins::DNS & PduDns)
{
  ssize_t Result = -1;
  uint16_t PduDnsSize = 0;

  Result = Connector->read_n(&PduDnsSize, sizeof(PduDnsSize));
  if (Result == -1)
   return Result;

  PduDnsSize = ntohs(PduDnsSize);
  std::vector<uint8_t> ReadBuffer(PduDnsSize);
  Result = Connector->read_n(ReadBuffer.data(), ReadBuffer.size());
  if (Result == -1)
    return Result;

  PduDns = Tins::DNS(ReadBuffer.data(), static_cast<uint32_t>(ReadBuffer.size()));

  return Result;
}


std::ostream & operator<<(std::ostream & os, Tins::DNS::QueryType queryType)
{
  /*
  switch (queryType)
  {
    case Tins::DNS::QueryType::A:
      std::cout << "A";
      break;

    case Tins::DNS::QueryType::NS:
      std::cout << "NS";
      break;
  }
  */

  std::cout << magic_enum::enum_name(queryType);

  return os;  
}


std::ostream & operator<<(std::ostream & os, Tins::DNS const & PduDns)
{
  std::cout << "Queries: " << PduDns.questions_count() << '\n';
  for (Tins::DNS::query const & qr : PduDns.queries())
    std::cout << "  " << qr.dname() << ' ' << qr.query_type() << '\n';

	std::cout << "Answ: " << PduDns.answers_count() << '\n';
  for (Tins::DNS::resource const & answ : PduDns.answers())
    std::cout << "  " << answ.dname() << ' ' << static_cast<Tins::DNS::QueryType>(answ.query_type()) << ' ' << answ.data() << '\n';

  std::cout << "Auth: " << PduDns.authority_count() << '\n';
  for (Tins::DNS::resource const & auth : PduDns.authority())
    std::cout << "  " << auth.dname() << ' ' << static_cast<Tins::DNS::QueryType>(auth.query_type()) << ' ' << auth.data() << '\n';

	std::cout << "Adds: " << PduDns.additional_count() << '\n';
  for (Tins::DNS::resource const & add : PduDns.additional())
    std::cout << "  " << add.dname() << ' ' << static_cast<Tins::DNS::QueryType>(add.query_type()) << ' ' << add.data() << '\n';

  return os;
}


void GatherInfoFromRRs(Tins::DNS::resources_type const & RRs, LocalCache & cache)
{
  for (Tins::DNS::resource const & res : RRs)
    switch (res.query_type())
    {
      case Tins::DNS::A:
        cache.addA(res.dname(), res.data(), res.ttl());
        break;
      
      case Tins::DNS::NS:
        cache.addNS(res.dname(), res.data(), res.ttl());
        break;
    }
}


void GatherInfoFromPduDns(Tins::DNS const & PduDns, LocalCache & cache)
{
  GatherInfoFromRRs(PduDns.answers(), cache);
  GatherInfoFromRRs(PduDns.authority(), cache);
  GatherInfoFromRRs(PduDns.additional(), cache);

  if (!PduDns.authority().empty() && PduDns.authority().front().query_type() == Tins::DNS::QueryType::SOA)
  {
    Tins::DNS::soa_record SoaRecord(PduDns.authority().front());
    cache.addNS(PduDns.queries().front().dname(), SoaRecord.mname(), SoaRecord.expire());
  }
}


void PrintZoneNSaddrs(std::string const & node, LocalCache & cache)
{
  for (std::string const & name : cache.getNS(node))
  {
    std::cout << name << " - ";
    if (cache.hasA(name))
    {
      std::cout << cache.getA(name);
    }
    else
    {
      std::cout << "unknown";
    }

    std::cout << '\n'; 
  }
}


void Resolver(std::string name, std::shared_ptr<sockpp::socket> answerSocket)
{
  std::cout << std::string(8, '=') << ' ';
  std::cout << name << ' ';
  std::cout << std::string(8, '=') << std::endl;

  LocalCache cache;
  DomainName dName(name);

  if (cache.hasA(dName.GetFullName()))
  {
    std::cout << "Resolved ip: " << cache.getA(dName.GetFullName()) << std::endl;
    return;
  }

  std::string ip{};
  size_t level = GetStartLevel(cache, dName, ip);
  if (level == 0)
  {
    std::cout << "No root NS servers ip" << std::endl;
    return;
  }
  uint16_t const port = 53;
  sockpp::inet_address NsAddr(ip, port);
  std::shared_ptr<sockpp::connector> Connector(new sockpp::connector(NsAddr));


  for (; level <= dName.GetLabelsCount(); level++)
  {
    Tins::DNS PduDns = makePduDnsQuerry(dName.GetName(level), Tins::DNS::QueryType::NS);
    ssize_t Result = -1;

    Result = SendPduDns(Connector, PduDns);
    Result = ReadPduDns(Connector, PduDns);

    std::cout << "Zone: " << dName.GetName(level) << '\n';
	  std::cout << PduDns << std::endl;
	  std::cout.flush();

    GatherInfoFromPduDns(PduDns, cache);

    for (std::string const & name : cache.getNS(dName.GetName(level)))
      if (!cache.hasA(name))
      {
        PduDns = makePduDnsQuerry(name, Tins::DNS::QueryType::A);

        Result = SendPduDns(Connector, PduDns);
        Result = ReadPduDns(Connector, PduDns);
        
        std::cout << "Ip inquiry for: " << name << '\n';
	      std::cout << PduDns << std::endl;
	      std::cout.flush();

        GatherInfoFromPduDns(PduDns, cache);
      }

    PrintZoneNSaddrs(dName.GetName(level), cache);
    std::cout << std::endl;
    
    bool isIpFound = false;
    for (std::string const & ns : cache.getNS(dName.GetName(level)))
      if (cache.hasA(ns))
      {
        ip = cache.getA(ns);
        isIpFound = true;
        break;
      }

    if (!isIpFound)
    {
      std::cout << "No ip of next NS server" << std::endl;
      return;
    }

    NsAddr.create(ip, port);
	  Connector->connect(NsAddr);
  }

  Tins::DNS PduDns = makePduDnsQuerry(dName.GetFullName(), Tins::DNS::QueryType::A);
  ssize_t Result = -1;
  Result = SendPduDns(Connector, PduDns);
  Result = ReadPduDns(Connector, PduDns);
  
  std::cout << "Query for A record (" << dName.GetFullName() << ")\n";
  std::cout << PduDns << std::endl;

  GatherInfoFromPduDns(PduDns, cache);

  std::cout << "Resolved ip: " << cache.getA(dName.GetFullName()) << std::endl;
}