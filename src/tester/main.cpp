#include <sockpp/connector.h>
#include <sockpp/inet_address.h>
#define TINS_STATIC
#include <tins/dns.h>
#include <iostream>


Tins::DNS makePduDnsQuery(std::string const & name, Tins::DNS::QueryType queryType)
{
  Tins::DNS::query query
  (
    name,
    queryType,
    Tins::DNS::QueryClass::INTERNET
  );

  Tins::DNS pdu{};

  pdu.id(1);
  pdu.type(Tins::DNS::QRType::QUERY);
  pdu.opcode(0);
  //pdu.recursion_desired(1);
  pdu.add_query(query);

  return pdu;
}


struct Tester
{
private:
  sockpp::connector socket;

public:
  explicit Tester(sockpp::inet_address const & address)
    : socket(address)
  {}

private:
  ssize_t sendPdu(Tins::DNS & pdu)
  {
    ssize_t result = -1;
    uint16_t size = htons(static_cast<uint16_t>(pdu.size()));

    result = socket.write_n(&size, sizeof(size));
    if (result == -1)
      return result;

    result = socket.write_n(pdu.serialize().data(), pdu.size());

    return result;
  }


  ssize_t readPdu(Tins::DNS & pdu)
  {
    ssize_t result = -1;
    uint16_t size = 0;

    result = socket.read_n(&size, sizeof(size));
    if (result == -1)
      return result;

    size = ntohs(size);
    std::vector<uint8_t> buffer(size);
    result = socket.read_n(buffer.data(), size);
    if (result == -1)
      return result;

    try
    {
      pdu = Tins::DNS(buffer.data(), size);
    }
    catch (...)
    {
      return -1;
    }

    return result;
  }

public:
  std::string getIp(std::string name)
  {
    Tins::DNS pdu = makePduDnsQuery(name, Tins::DNS::QueryType::A);
    if (sendPdu(pdu) == -1 || readPdu(pdu) == -1)
    {
      return "error";
    }

    return pdu.answers().front().data();
  }
};


int main()
{
  sockpp::socket_initializer socketInit;

  while (true)
  {
    std::string name;
    std::cin >> name;

    if (name == "exit")
    {
      break;
    }

    Tester tester(sockpp::inet_address("127.0.0.1", 30000));
    std::string result = tester.getIp(name);
    std::cout << result << '\n' << std::endl;
  }

  return 0;
}