#include <iostream>
#include "Resolver.h"
#include "Receiver.h"

int main()
{
  sockpp::socket_initializer SocketInit;

  /*
  Resolver resolver("music.yandex.ru", Tins::DNS::QueryType::A, Tins::DNS::QueryClass::INTERNET);

  std::string result;
  if (resolver.resolve() == 0)
  {
    result = resolver.getAddress();
  }
  else
  {
    result = "error";
  }

  std::cout << result << std::endl;
  */

  Receiver receiver(30000);

  return receiver.run();
}
