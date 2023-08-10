#include "Resolver.h"
#include <algorithm>
#include <cctype>
#include "SocketExceptions.h"
#include <random>


uint16_t const dnsPort = 53;

size_t const rootNsCount = 13;

std::vector<std::string> const rootNsAddrs =
{
  "198.41.0.4",
  "199.9.14.201",
  "192.33.4.12",
  "199.7.91.13",
  "192.203.230.10",
  "192.5.5.241",
  "192.112.36.4",
  "198.97.190.53",
  "192.36.148.17",
  "192.58.128.30",
  "193.0.14.129",
  "199.7.83.42",
  "202.12.27.33"
};


std::string const & Resolver::getAddress() const
{
  return answerAddr;
}


int Resolver::resolve()
{
  serverAddr.create(getRootNsAddr(), dnsPort);

  while (!queryStack.empty())
  {
    Tins::DNS pdu = sendAndReceiveQuery();
    Query const & query = queryStack.back();
    bool success = false;

    for (auto const & answer : pdu.answers())
    {
      if (!isSameName(answer.dname(), query.sName))
      {
        continue;
      }

      if (answer.query_type() == query.qType)
      {
        serverAddr.create(answer.data(), dnsPort);
        queryStack.pop_back();
        if (queryStack.empty())
        {
          answerAddr = answer.data();
        }
        success = true;
        break;
      }
      
      if (answer.query_type() == Tins::DNS::QueryType::CNAME)
      {
        serverAddr.create(getRootNsAddr(), dnsPort);
        Tins::DNS::QueryType type = query.qType;
        queryStack.pop_back();
        queryStack.emplace_back(answer.data(), type);
        success = true;
        break;
      }
      
    }

    if (success)
    {
      continue;
    }

    if (pdu.authority_count() == 0 || 
      pdu.authority().front().query_type() != Tins::DNS::QueryType::NS)
    {
      return -1;
    }

    std::string nsName = pdu.authority().front().data();
    for (auto const & additional : pdu.additional())
    {
      if (isSameName(additional.dname(), nsName) && additional.query_type() == Tins::DNS::QueryType::A)
      {
        serverAddr.create(additional.data(), dnsPort);
        success = true;
        break;
      }
    }

    if (success)
    {
      continue;
    }

    serverAddr.create(getRootNsAddr(), dnsPort);
    queryStack.emplace_back(nsName, Tins::DNS::QueryType::A);
  }

  return 0;
}


std::string const & Resolver::getRootNsAddr() const
{
  static size_t callNo = std::rand() % rootNsCount;

  return rootNsAddrs[(callNo++) % rootNsCount];
}


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


void Resolver::sendPdu(Tins::DNS & pdu)
{
  uint16_t size = htons(static_cast<uint16_t>(pdu.size()));
  /*
  if (socket.write_n(&size, sizeof(size)) == -1 ||
    socket.write_n(pdu.serialize().data(), pdu.size()) == -1)
  {
    throw sock_exception(&socket);
  }
  */
  
  std::vector<uint8_t> buffer(2);
  buffer.reserve(pdu.size() + 2);
  *reinterpret_cast<uint16_t *>(&buffer.front()) = size;
  auto pduData = pdu.serialize();
  buffer.insert(buffer.end(), pduData.begin(), pduData.end());

  if (socket.write_n(buffer.data(), buffer.size()) == -1)
  {
    throw sock_exception(&socket);
  }
  
}


Tins::DNS Resolver::readPdu()
{
  uint16_t size = 0;

  if (socket.read_n(&size, sizeof(size)) == -1)
  {
    throw sock_exception(&socket);
  }

  size = ntohs(size);
  if (size == 0)
  {
    throw std::exception("zero-length pdu");
  }

  std::vector<uint8_t> buffer(size);

  if (socket.read_n(buffer.data(), size) == -1)
  {
    throw sock_exception(&socket);
  }

  return Tins::DNS(buffer.data(), size);
}


Tins::DNS Resolver::sendAndReceiveQuery()
{
  Query const & query = queryStack.back();
  Tins::DNS pdu = makePduDnsQuery(query.sName, query.qType);
  auto buffer = pdu.serialize();

  if (!socket.connect(serverAddr))
  {
    throw sock_exception(&socket);
  }

  sendPdu(pdu);

  return readPdu();
}


bool isSameName(std::string const & lhs, std::string const & rhs)
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
    [](char lhs, char rhs) -> bool
    {
      return std::tolower(lhs) == std::tolower(rhs);
    }
  );
}
