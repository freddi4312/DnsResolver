// DnsResolver.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <list>
#include <limits>
#include <memory>

#include "DnsResolver.h"
#include <sockpp/socket.h>
#include <sockpp/connector.h>
#include <sockpp/inet_address.h>
#define TINS_STATIC
#include <tins/dns.h>
#include "Resolver.h"
#include "GlobalCache.h"


/*
Tins::DNS GetDnsRequestToRoot(std::string const &Label)
{
	Tins::DNS PduDns{};
	PduDns.id(30);
	PduDns.type(Tins::DNS::QRType::QUERY);
	PduDns.opcode(0);
	Tins::DNS::Query DnsQuery(Label, Tins::DNS::QueryType::NS, Tins::DNS::QueryClass::IN);
	PduDns.add_query(DnsQuery);

	return PduDns;
}
*/

/*
struct NsResolve
{
public:
  explicit NsResolve(std::string const & name) :
    _isResolved(false),
	_name(name),
	_ip()
  {}

  void Resolve(std::string const & ip)
  {
	_ip = ip;
	_isResolved = true;
  }

  std::string const & GetName()
  {
	return _name;
  }

  bool isResolved()
  {
	return _isResolved;
  }

  std::string const & GetIp()
  {
	return _ip;
  }

private:
  bool _isResolved;
  std::string _name;
  std::string _ip;
};
*/

/*
struct NsResolve
{
public:
  NsResolve() :
    _isResolved(false),
	_ip()
  {}

  void Resolve(std::string const & ip)
  {
	_ip = ip;
	_isResolved = true;
  }

  bool isResolved() const
  {
	return _isResolved;
  }

  std::string const & GetIp() const
  {
	return _ip;
  }

private:
  bool _isResolved;
  std::string _ip;
};


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

  size_t GetLabelsCount()
  {
	return _labels.size();
  }

  std::string GetName(size_t level)
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

  std::string GetFullName()
  {
	return GetName(_labels.size());
  }

private:
  std::list<std::string> _labels;
};


struct NamesCompare
{
  bool operator()(std::string const & lhs, std::string const & rhs) const
  {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), 
	                                    rhs.begin(), rhs.end(),
										[](char l, char r)
										{
                                          return std::tolower(l) < std::tolower(r);
										});
  }
};


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
  PduDns.add_query(Query);

  return PduDns;
}


std::string ToLowerCase(std::string str)
{
  std::transform(str.begin(), str.end(), str.begin(), [](char c){ return std::tolower(c); });

  return str;
}
*/

int main()
{

#ifdef _MSVC_LANG
  std::cout << _MSVC_LANG << std::endl;
#else
  std::cout << __cplusplus << std::endl;
#endif // MSVC_CPLUSPLSU
  
  sockpp::socket_initializer SocketInit{};

  GlobalCache &globalCache = GlobalCache::getInstance();
  uint32_t const ttl_max = (std::numeric_limits<uint32_t>::max)();
  globalCache.addNS("", "a.root-servers.net", ttl_max);
  globalCache.addA("a.root-servers.net", "198.41.0.4", ttl_max);

  std::string name1("twitch.tv");
  std::shared_ptr<sockpp::socket> dummySocket1(new sockpp::socket());
  Resolver(std::move(name1), std::move(dummySocket1));
/*
  std::string name2("google.com");
  std::shared_ptr<sockpp::socket> dummySocket2(new sockpp::socket());
  Resolver(std::move(name2), std::move(dummySocket2));
  */
/*
  std::string name3("music.yandex.ru");
  std::shared_ptr<sockpp::socket> dummySocket3(new sockpp::socket());
  Resolver(std::move(name3), std::move(dummySocket3));
*/
  return 0;
}

/*
int main_2()
{
  sockpp::socket_initializer SocketInit{};

  uint16_t const port = 53;
  sockpp::inet_address NsAddr("e.root-servers.net", port);
  sockpp::connector Connector(NsAddr);

  std::cout << "Domain name: ";
  std::string line{};
  std::cin >> line;
  std::cout << std::endl;
  DomainName dName(line);

  for (size_t level = 1; level <= dName.GetLabelsCount(); level++)
  {
    Tins::DNS PduDns = makePduDnsQuerry(dName.GetName(level), Tins::DNS::QueryType::NS);
    ssize_t Result = -1;
    uint16_t PduDnsSize = htons(static_cast<uint16_t>(PduDns.size()));
    Result = Connector.write_n(&PduDnsSize, sizeof(PduDnsSize));
    Result = Connector.write_n(PduDns.serialize().data(), PduDns.size());
  
    Result = Connector.read_n(&PduDnsSize, sizeof(PduDnsSize));
    PduDnsSize = ntohs(PduDnsSize);
    std::vector<uint8_t> ReadBuffer(PduDnsSize);
    Result = Connector.read_n(ReadBuffer.data(), ReadBuffer.size());
  
    PduDns = Tins::DNS(ReadBuffer.data(), ReadBuffer.size());
    
	  std::cout << "Zone: " << dName.GetName(level) << '\n';
	  std::cout << "Answ: " << PduDns.answers_count() << '\n';
    std::cout << "Auth: " << PduDns.authority_count() << '\n';
	  std::cout << "Adds: " << PduDns.additional_count() << '\n';
	  std::cout.flush();
  
    std::map<std::string, NsResolve, NamesCompare> NsResolving{};
    for (auto const & res: PduDns.authority())
      NsResolving.emplace(res.data(), NsResolve{});
  
    bool isSmthResolved = false;
    for (auto const & res : PduDns.additional())
      if (res.query_type() == Tins::DNS::QueryType::A)
  	  {
  	    NsResolving.at(res.dname()).Resolve(res.data());
  	    isSmthResolved = true;
  	  }
  
    if (isSmthResolved == false)
    {
  	  auto It = NsResolving.begin();
  	  PduDns = makePduDnsQuerry(It->first, Tins::DNS::QueryType::A);
  
      PduDnsSize = htons(static_cast<uint16_t>(PduDns.size()));
      Result = Connector.write_n(&PduDnsSize, sizeof(PduDnsSize));
      Result = Connector.write_n(PduDns.serialize().data(), PduDns.size());
  
  	  Result = Connector.read_n(&PduDnsSize, sizeof(PduDnsSize));
      PduDnsSize = ntohs(PduDnsSize);
      ReadBuffer.resize(PduDnsSize);
      Result = Connector.read_n(ReadBuffer.data(), ReadBuffer.size());
  
  	  PduDns = Tins::DNS(ReadBuffer.data(), ReadBuffer.size());
  
  	  It->second.Resolve(PduDns.answers().front().data());
    }
  
  
    for (auto const & p : NsResolving)
    {
  	  std::cout << p.first << " - ";
  	  if (p.second.isResolved())
  	    std::cout << p.second.GetIp();
      else
  	    std::cout << "unknown";
  	  std::cout << "\n";
    }
    std::cout << std::endl;  


    std::string ip{};
    for (auto const & p : NsResolving)
	  {
      if (p.second.isResolved())
	    ip = p.second.GetIp();
	  }

	  NsAddr.create(ip, port);
	  Connector.connect(NsAddr);
  }


  Tins::DNS PduDns = makePduDnsQuerry(dName.GetFullName(), Tins::DNS::QueryType::A);

  uint16_t PduDnsSize = htons(static_cast<uint16_t>(PduDns.size()));
  ssize_t Result = -1;
  Result = Connector.write_n(&PduDnsSize, sizeof(PduDnsSize));
  Result = Connector.write_n(PduDns.serialize().data(), PduDns.size());  

  Result = Connector.read_n(&PduDnsSize, sizeof(PduDnsSize));
  PduDnsSize = ntohs(PduDnsSize);
  std::vector<uint8_t> ReadBuffer(PduDnsSize);
  Result = Connector.read_n(ReadBuffer.data(), ReadBuffer.size());  

  PduDns = Tins::DNS(ReadBuffer.data(), ReadBuffer.size());

  std::cout << "Resolved addresses:\n";
  for (auto const & res : PduDns.answers())
  {
    std::cout << res.data() << '\n';
  }
  std::cout.flush();

  return 0;
}
*/

int main_1()
{
	sockpp::socket_initializer SocketInit{};

	std::vector<std::string> RootIP{"198.41.0.4", "199.9.14.201"};
	/*
	std::ifstream fin("RootIP.txt");
	std::string str{};
	while (std::getline(fin, str, '\n'))
		RootIP.push_back(str);
  */
	//Resolver DnsResolver(RootIP);


  std::string DomainName;
  
  //std::cin >> DomainName;
	DomainName = "music.yandex.ru";

  //std::string ResolvedIP = DnsResolver.resolve(DomainName);
  //std::cout << "Resolved IP: " << ResolvedIP << std::endl;


	/*
	Tins::DNS PduDns = GetDnsRequestToRoot("com");
	ssize_t Count = 0;

	sockpp::connector TcpConnector{};
	sockpp::inet_address RootAddr("193.0.14.129", 53);

	if (TcpConnector.connect(RootAddr))
		std::cout << "Connected to " << RootAddr.to_string() << std::endl;
	else
		std::cout << "Connection failed" << std::endl;

	uint16_t MessageSize = htons(PduDns.size());
	Count = TcpConnector.write_n(&MessageSize, sizeof(uint16_t));
	std::cout << "Send " << Count << " out of " << sizeof(uint16_t) << " bytes" << std::endl;

	Count = TcpConnector.write_n(PduDns.serialize().data(), PduDns.size());
	std::cout << "Send " << Count << " out of " << PduDns.size() << " bytes" << std::endl;

	uint16_t PduSize = 0;
	Count = TcpConnector.read_n(&PduSize, sizeof(PduSize));
	if (Count != -1)
	{
		PduSize = ntohs(PduSize);
		std::cout << "Read " << Count << " bytes\n";
		std::cout << "Pdu size = " << PduSize << std::endl;
	}
	else
	{
		std::cout << "Error " << TcpConnector.last_error() << ": " << TcpConnector.last_error_str() << std::endl;
	}

	std::vector<uint8_t> PduBuffer(PduSize);
	Count = TcpConnector.read_n(PduBuffer.data(), PduBuffer.size());
	std::cout << "Read " << Count << " bytes\n";

	try
	{
		PduDns = Tins::DNS(PduBuffer.data(), PduBuffer.size());
		std::cout << "Name servers count = " << PduDns.authority_count() << std::endl;
	}
	catch (Tins::malformed_packet)
	{
		std::cout << "Malformed packet" << std::endl;
	}
	*/

	return 0;
}
