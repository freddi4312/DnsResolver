#include "common.h"
#include <tins/dns.h>
#include <sockpp/stream_socket.h>
#include <sockpp/datagram_socket.h>


struct IDnsXmitter
{
public:
  virtual void send(Tins::DNS & pdu) = 0;
  virtual auto receive() -> Tins::DNS = 0;
  virtual ~IDnsXmitter() = default;
};


struct TcpDnsXmitter : public IDnsXmitter
{
private:
  sockpp::stream_socket socket;

public:
  TcpDnsXmitter(sockpp::stream_socket && socket)
    : socket(std::move(socket))
  {}
  virtual void send(Tins::DNS & pdu);
  virtual auto receive() -> Tins::DNS;
};


struct UdpDnsXmitter : public IDnsXmitter
{
private:
  sockpp::datagram_socket socket;

public:
  UdpDnsXmitter(sockpp::datagram_socket && socket)
    : socket(std::move(socket))
  {}
  virtual void send(Tins::DNS & pdu);
  virtual auto receive() -> Tins::DNS;
};
