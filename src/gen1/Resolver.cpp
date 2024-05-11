#include "Resolver.h"
#include <algorithm>
#include <cctype>
#include "SocketExceptions.h"
#include <random>
#include "Cache.h"


uint16_t const dnsPort = 53;


auto Resolver::getAddress() const -> std::vector<std::string>
{
  return answerAddr;
}


auto Resolver::getCanonicalNames() const -> std::vector<std::pair<std::string, std::string>>
{
  return canonicalNames;
}


auto Resolver::resolve() -> int
{
  Cache & cache = Cache::getInstance();

  while (!queryStack.empty())
  {
    if (queryStack.top().type != Tins::DNS::A)
    {
      return -1;
    }

    fillQueryFromCache();
    Query & query = queryStack.top();
    if (!query.answer.empty())
    {
      queryStack.pop();
      continue;
    }

    if (query.nsServers.empty())
    {
      return -1; // no root name servers in cache;
    }

    bool is_next_data_obtained = false;
    bool is_answer_obtained = false;

    for (auto server_it  = query.nsServers.begin(); server_it != query.nsServers.end(); )
    {
      if (server_it->ip.empty())
      {
        Query new_query(server_it->name, Tins::DNS::A, server_it->ip);
        queryStack.push(std::move(new_query));
        break;
      }

      for (std::string const & ip : server_it->ip)
      {
        serverAddr.create(ip, dnsPort);
        Tins::DNS pdu = sendAndReceiveQuery();
        cache.add(pdu);

        for (auto const & answer : pdu.answers())
        {
          if (!isSameName(answer.dname(), query.name))
          {
            continue;
          }

          if (answer.query_type() == Tins::DNS::A
            && isSameName(answer.dname(), query.name))
          {
            query.answer.push_back(answer.data());
            is_answer_obtained = true;
          }

          if (answer.query_type() == Tins::DNS::CNAME)
          {
            std::string cname = answer.data();
            canonicalNames.emplace_back(query.name, cname);
            Query new_query(cname, query.type, query.answer);
            queryStack.pop();
            queryStack.push(std::move(new_query));
            is_next_data_obtained = true;
            break;
          }
        }

        if (is_answer_obtained || is_next_data_obtained)
        {
          break;
        }

        std::list<Query::Server> new_ns_servers;
        for (auto const & name_rsc : pdu.authority())
        {
          if (name_rsc.query_type() != Tins::DNS::NS)
          {
            continue;
          }

          std::vector<std::string> ip_list;
          for (auto const & ip_rsc : pdu.additional())
          {
            if (isSameName(ip_rsc.dname(), name_rsc.data()) && ip_rsc.query_type() == Tins::DNS::QueryType::A)
            {
              ip_list.push_back(ip_rsc.data());
            }
          }

          if (ip_list.empty())
          {
            new_ns_servers.emplace_back(name_rsc.data(), ip_list);
          }
          else
          {
            new_ns_servers.emplace_front(name_rsc.data(), std::move(ip_list));
          }
        }

        if (!new_ns_servers.empty())
        {
          is_next_data_obtained = true;
          query.nsServers = std::move(new_ns_servers);
          break;
        }
      }

      if (is_answer_obtained || is_next_data_obtained)
      {
        break;
      }
    }

    if (is_answer_obtained)
    {
      queryStack.pop();
    }
    else if (!is_next_data_obtained && queryStack.empty())
    {
      return -1;
    }
  }

  return 0;
}


void Resolver::fillQueryFromCache()
{
  Cache & cache = Cache::getInstance();
  Query & query = queryStack.top();

  while (true)
  {
    auto result = cache.get(Tins::DNS::CNAME, query.name);
    if (result.empty())
    {
      break;
    }

    std::string cname = result.front();
    if (queryStack.size() == 1)
    {
      canonicalNames.emplace_back(query.name, cname);
    }

    query.name = cname;
  }

  if (query.nsServers.empty())
  {
    query.answer = cache.get(Tins::DNS::A, query.name);
    if (!query.answer.empty())
    {
      return;
    }

    auto ns_servers = cache.get(Tins::DNS::NS, query.name);
    for (std::string & server_name : ns_servers)
    {
      query.nsServers.push_back(Query::Server(std::move(server_name), {}));
    }
  }

  std::list<Query::Server> no_ip_servers;

  for (auto server_it = query.nsServers.begin(); server_it != query.nsServers.end();)
  {
    if (server_it->ip.empty())
    {
      server_it->ip = cache.get(Tins::DNS::A, server_it->name);
    }

    if (server_it->ip.empty())
    {
      no_ip_servers.emplace_back(std::move(*server_it));
      server_it = query.nsServers.erase(server_it);
    }
    else
    {
      ++server_it;
    }
  }

  query.nsServers.splice(query.nsServers.end(), no_ip_servers);
}


/*
auto Resolver::resolve() -> int
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
*/

/*
auto Resolver::getRootNsAddr() const -> std::string const &
{
  static size_t callNo = std::rand() % rootNsCount;

  return rootNsAddrs[(callNo++) % rootNsCount];
}
*/

auto makePduDnsQuery(std::string const & name, Tins::DNS::QueryType queryType) -> Tins::DNS
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


auto Resolver::readPdu() -> Tins::DNS
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


auto Resolver::sendAndReceiveQuery() -> Tins::DNS
{
  Query const & query = queryStack.top();
  Tins::DNS pdu = makePduDnsQuery(query.name, query.type);
  auto buffer = pdu.serialize();

  if (!socket.connect(serverAddr))
  {
    throw sock_exception(&socket);
  }

  sendPdu(pdu);

  return readPdu();
}


auto isSameName(std::string const & lhs, std::string const & rhs) -> bool
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
    [](char lhs, char rhs) -> bool
    {
      return std::tolower(lhs) == std::tolower(rhs);
    }
  );
}
