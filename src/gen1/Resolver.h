#ifndef RESOLVER_H
#define RESOLVER_H


#include "common.h"
#include <tins/dns.h>
#include <sockpp/connector.h>
#include <sockpp/inet_address.h>
#include <string>
#include <vector>
#include <exception>
#include <list>
#include <stack>

/*
extern size_t const rootNsCount;
extern std::vector<std::string> const rootNsAddrs;
*/

auto isSameName(std::string const & lhs, std::string const & rhs) -> bool;


struct Query
{
public:
  struct Server
  {
    Server(std::string name, std::vector<std::string> ip)
      : name(std::move(name)), ip(std::move(ip))
    {}

    std::string name;
    std::vector<std::string> ip;
  };

  Query(std::string const & name, Tins::DNS::QueryType type, std::vector<std::string> & answer)
    : name(name), type(type), nsServers(), answer(answer)
  {}
  
  std::string name;
  Tins::DNS::QueryType type;
  std::list<Server> nsServers;
  std::vector<std::string> & answer;
};


struct Resolver
{
private:
  std::stack<Query> queryStack;
  Tins::DNS::QueryClass qClass;
  sockpp::inet_address serverAddr;
  sockpp::connector socket;
  std::vector<std::string> answerAddr;
  std::vector<std::pair<std::string, std::string>> canonicalNames;
  

public:
  Resolver(std::string const & sName, Tins::DNS::QueryType qType, Tins::DNS::QueryClass qClass)
    : queryStack(), qClass(qClass)
  {
    queryStack.emplace(sName, qType, answerAddr);
  }

  auto resolve() -> int;
  auto getAddress() const -> std::vector<std::string>;
  auto getCanonicalNames() const -> std::vector<std::pair<std::string, std::string>>;

private:
  void fillQueryFromCache();
  //auto getRootNsAddr() const -> std::string const &;
  void sendPdu(Tins::DNS & pdu);
  auto readPdu() -> Tins::DNS;
  auto sendAndReceiveQuery() -> Tins::DNS;
  //auto handlePdu(Tins::DNS const & pdu) const -> int;
};


#endif // !RESOLVER_H
