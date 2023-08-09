#include "Receiver.h"
#include <thread>
#include <exception>


Receiver::Receiver(uint16_t port)
  : server(sockpp::inet_address("127.0.0.1", dnsPort))
{}


int Receiver::run()
{
  if (!server.listen())
  {
    return -1;
  }

  while (true)
  {
    sockpp::stream_socket client = server.accept();
    std::thread thread
    (
      [this](sockpp::stream_socket && client) 
      {
        handleClient(std::move(client)); 
      },
      std::move(client)
    );

    thread.detach();
  }
}


void Receiver::handleClient(sockpp::stream_socket && client)
{
  ssize_t result = -1;
  uint16_t size = 0;

  try
  {
    result = client.read_n(&size, sizeof(size));
    if (result == -1)
      throw std::exception("read size = 0");
    
    size = ntohs(size);
    std::vector<uint8_t> buffer(size);
    result = client.read_n(buffer.data(), size);
    if (result == -1)
      throw std::exception("couldn't read data");

    Tins::DNS pdu(buffer.data(), size);

    for (auto const & query : pdu.queries())
    {
      if (query.query_class() != Tins::DNS::QueryClass::INTERNET ||
        query.query_type() != Tins::DNS::QueryType::A)
      {
        continue;
      }

      Resolver resolver(query.dname(), Tins::DNS::QueryType::A, Tins::DNS::QueryClass::INTERNET);
      if (resolver.resolve() != 0)
      {
        std::string const msg = "couldn't resolve " + query.dname();
        throw std::exception(msg.c_str());
      }

      std::cout << "resolved " + query.dname() + " to " + resolver.getAddress() << std::endl;

      Tins::DNS::resource const answer
      (
        query.dname(),
        resolver.getAddress(),
        Tins::DNS::QueryType::A,
        Tins::DNS::QueryClass::INTERNET,
        6000
      );

      pdu.add_answer(answer);
    }

    pdu.type(Tins::DNS::QRType::RESPONSE);

    size = htons(static_cast<uint16_t>(pdu.size()));

    result = client.write_n(&size, sizeof(size));
    if (result == -1)
      throw std::exception("size write error");

    result = client.write_n(pdu.serialize().data(), pdu.size());
    if (result == -1)
      throw std::exception("data write error");
  }
  catch (std::exception & e)
  {
    std::cout << e.what() << std::endl;
  }
}
