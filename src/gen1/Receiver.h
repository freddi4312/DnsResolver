#ifndef RECEIVER_H
#define RECEIVER_H


#include "Resolver.h"
#include <sockpp/acceptor.h>
#include <sockpp/datagram_socket.h>
#include <sockpp/inet_address.h>
#include "DnsXmitter.h"


struct TcpReceiver
{
private:
  sockpp::acceptor server;

public:
  explicit TcpReceiver(uint16_t port = dnsPort)
    : server(sockpp::inet_address("127.0.0.1", port))
  {}
  auto run() -> int;
};


struct UdpReceiver
{
private:
  //sockpp::datagram_socket server;
  sockpp::inet_address serverAddr;

public:
  explicit UdpReceiver(uint16_t port = dnsPort)
    : serverAddr("127.0.0.1", port)
  {}
  auto run() -> int;
};


void handleClient(Tins::DNS pdu, IDnsXmitter && client);
std::string ipListToString(std::vector<std::string> const & ip_list);


#endif // !RECEIVER_H
