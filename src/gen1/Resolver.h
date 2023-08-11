#ifndef RESOLVER_H
#define RESOLVER_H


#include "common.h"
#include <tins/dns.h>
#include <sockpp/connector.h>
#include <sockpp/inet_address.h>
#include <string>
#include <vector>
#include <exception>


extern size_t const rootNsCount;
extern std::vector<std::string> const rootNsAddrs;


auto isSameName(std::string const & lhs, std::string const & rhs) -> bool;


struct Query
{
public:
  Query(std::string const & sName, Tins::DNS::QueryType qType)
    : sName(sName), qType(qType)
  {}
  
  std::string const sName;
  Tins::DNS::QueryType const qType;
};


struct Resolver
{
private:
  std::vector<Query> queryStack;
  Tins::DNS::QueryClass qClass;
  sockpp::inet_address serverAddr;
  sockpp::connector socket;
  std::string answerAddr;
  

public:
  Resolver(std::string const & sName, Tins::DNS::QueryType qType, Tins::DNS::QueryClass qClass)
    : queryStack{ {sName, qType} }, qClass(qClass)
  {}

  auto resolve() -> int;
  auto getAddress() const -> std::string const &;

private:
  auto getRootNsAddr() const -> std::string const &;
  void sendPdu(Tins::DNS & pdu);
  auto readPdu() -> Tins::DNS;
  auto sendAndReceiveQuery() -> Tins::DNS;
  auto handlePdu(Tins::DNS const & pdu) const -> int;
};


#endif // !RESOLVER_H
