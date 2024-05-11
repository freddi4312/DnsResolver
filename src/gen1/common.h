#ifndef COMMON_H
#define COMMON_H

#define TINS_STATIC

#include <cstdint>
#include <string>

extern uint16_t const dnsPort;

extern size_t const udpMaxPduSize;

auto isSameName(std::string const & lhs, std::string const & rhs) -> bool;
auto adapt(std::string const & name) -> std::string;

#endif // !COMMON_H
