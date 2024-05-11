#ifndef RECORD_H
#define RECORD_H


#include "common.h"
#include <string>
#include <chrono>
#include <Tins/dns.h>
#include <type_traits>


struct RecordName
{
  Tins::DNS::QueryType type;
  std::string name;

  RecordName(Tins::DNS::QueryType type, const std::string & name)
    : type(type), name(name)
  {}
};


struct Record
{
  std::string data;
  std::chrono::system_clock::time_point valid_until;

  Record(const std::string & data, const std::chrono::system_clock::time_point & valid_until)
    : data(data), valid_until(valid_until)
  {}
};


template<>
struct std::hash<RecordName>
{
  size_t operator()(RecordName const & key) const
  {
    return std::hash<int>{}(static_cast<int>(key.type)) ^ std::hash<std::string>{}(key.name);
  }
};


template<>
struct std::equal_to<RecordName>
{
  bool operator()(RecordName const & lhs, RecordName const & rhs) const
  {
    return lhs.type == rhs.type && lhs.name == rhs.name;
  }
};


#endif // !RECORD_H
