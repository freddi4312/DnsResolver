#include "DnsXmitter.h"
#include "SocketExceptions.h"
#include <vector>
#include <sockpp/inet_address.h>
#include <algorithm>


size_t const udpMaxPduSize = 512;


void TcpDnsXmitter::send(Tins::DNS & pdu)
{
  uint16_t size = htons(static_cast<uint16_t>(pdu.size()));
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


auto TcpDnsXmitter::receive() -> Tins::DNS
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


void UdpDnsXmitter::send(Tins::DNS & pdu)
{
  if (pdu.size() > udpMaxPduSize)
  {
    pdu.truncated(true);
  }

  std::vector<uint8_t> buffer = pdu.serialize();
  size_t sentSize = 0;

  while (sentSize < buffer.size())
  {
    size_t const packetSize = (std::min)(udpMaxPduSize, buffer.size() - sentSize);
    if (socket.send(buffer.data() + sentSize, packetSize) != packetSize)
    {
      throw sock_exception(&socket);
    }

    sentSize += packetSize;
  }
}


auto UdpDnsXmitter::receive() -> Tins::DNS
{
  static std::vector<uint8_t> buffer(udpMaxPduSize, 0);
  sockpp::inet_address srcAddr;

  ssize_t size = socket.recv_from(buffer.data(), buffer.size(), &srcAddr);
  if (size == -1)
  {
    throw sock_exception(&socket);
  }

  if (!socket.connect(srcAddr))
  {
    throw sock_exception(&socket);
  }

  return Tins::DNS(buffer.data(), size);
}
