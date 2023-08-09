#include "Resolver.h"
#include <sockpp/acceptor.h>

struct Receiver
{
private:
  sockpp::acceptor server;

public:
  explicit Receiver(uint16_t port = dnsPort);

  int run();

private:
  void handleClient(sockpp::stream_socket && client);
};
