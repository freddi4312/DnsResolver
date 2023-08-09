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


bool isSameName(std::string const & lhs, std::string const & rhs);


struct sock_exception : public std::exception
{
  using std::exception::exception;

  explicit sock_exception(sockpp::socket const * socket)
    : std::exception(socket->last_error_str().c_str())
  {}
};


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

  int resolve();
  std::string const & getAddress() const;

private:
  std::string const & getRootNsAddr() const;
  void Resolver::sendPdu(Tins::DNS & pdu);
  Tins::DNS Resolver::readPdu();
  Tins::DNS sendAndReceiveQuery();
  int handlePdu(Tins::DNS const & pdu) const;
};


#endif // !RESOLVER_H
