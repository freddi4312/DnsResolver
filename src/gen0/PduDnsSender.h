#pragma once


#define TINS_STATIC
#include <tins/dns.h>
#include <sockpp/stream_socket.h>
#include <sockpp/datagram_socket.h>


struct IPduDnsSender
{
    virtual bool SendPduDns(Tins::DNS & PduDns) = 0;
    virtual ~IPduDnsSender() = default;
};


struct stream_socket_dns : public sockpp::stream_socket, public IPduDnsSender
{
public:
  using sockpp::stream_socket::stream_socket;

  virtual bool SendPduDns(Tins::DNS & PduDns) override
  {
    uint16_t PduDnsSize = htons(static_cast<uint16_t>(PduDns.size()));
    
    if (write_n(&PduDnsSize, sizeof(PduDnsSize)) == -1)
      return false;

    if (write_n(PduDns.serialize().data(), PduDns.size()) == -1)
      return false;

    return true;
  }
};


struct datagram_socket_dns : public sockpp::datagram_socket, public IPduDnsSender
{
public:
  using sockpp::datagram_socket::datagram_socket;

  virtual bool SendPduDns(Tins::DNS & PduDns) override
  {
    if (send(PduDns.serialize().data(), PduDns.size()) == -1)
      return false;

    return true;
  }
};