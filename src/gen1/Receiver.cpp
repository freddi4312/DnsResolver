#include "Receiver.h"
#include <thread>
#include <exception>
#include <magic_enum.hpp>
#include <typeinfo>


auto TcpReceiver::run() -> int
{
  if (!server.listen())
  {
    return -1;
  }

  while (true)
  {
    TcpDnsXmitter client = server.accept();
    Tins::DNS pdu;

    try
    {
       pdu = client.receive();
    }
    catch (std::exception & e)
    {
      std::cout << e.what() << std::endl;
      continue;
    }

    std::thread thread(handleClient, std::move(pdu), std::move(client));
    thread.detach();
    //handleClient(std::move(pdu), std::move(client));

    /*
    std::thread thread
    (
      [this](sockpp::stream_socket && client) 
      {
        handleClient(std::move(client)); 
      },
      std::move(client)
    );

    thread.detach();
    */
  }

  return 0;
}


auto UdpReceiver::run() -> int
{
  while (true)
  {

    //UdpDnsXmitter client = sockpp::datagram_socket(server.clone().release());
    UdpDnsXmitter client = sockpp::datagram_socket(serverAddr);
    Tins::DNS pdu;

    try
    {
      pdu = client.receive();
    }
    catch (std::exception & e)
    {
      std::cout << typeid(e).name() << ": " << e.what() << std::endl;
      continue;
    }

    //std::thread thread(handleClient, std::move(pdu), std::move(client));
    //thread.detach();
    handleClient(std::move(pdu), std::move(client));
  }

  return 0;
}


void handleClient(Tins::DNS pdu, IDnsXmitter && client)
{
  ssize_t result = -1;
  uint16_t size = 0;
  bool hasUnsupportedQuery = false;

  try
  {
    for (auto const & query : pdu.queries())
    {
      if (query.query_class() != Tins::DNS::QueryClass::INTERNET ||
        query.query_type() != Tins::DNS::QueryType::A)
      {
        std::cout << "unsupported query type: " << magic_enum::enum_name(query.query_type()) << std::endl;
        hasUnsupportedQuery = true;
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
    pdu.recursion_available(true);
    if (hasUnsupportedQuery)
    {
      pdu.rcode(4);
    }

    client.send(pdu);
  }
  catch (std::exception & e)
  {
    std::cout << typeid(e).name() << ": " << e.what() << std::endl;
  }
}
