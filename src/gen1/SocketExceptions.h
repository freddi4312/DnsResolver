#ifndef SOCKETEXCEPTIONS_H
#define SOCKETEXCEPTIONS_H


#include <exception>
#include <sockpp/socket.h>


struct sock_exception : public std::exception
{
  using std::exception::exception;

  explicit sock_exception(sockpp::socket const * socket)
    : std::exception(socket->last_error_str().c_str())
  {}
};

#endif // !SOCKETEXCEPTIONS_H
