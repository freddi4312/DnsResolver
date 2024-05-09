#ifndef CACHE_H
#define CACHE_H


#include "common.h"
#include <unordered_map>
#include <string>
#include <Tins/dns.h>
#include <chrono>
#include <vector>
#include <list>
#include <random>
#include "DomainName.h"


class Cache
{
private:
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

  std::unordered_map<RecordName, std::list<Record>> records_;
  std::mt19937 generator_;

public:
  void add(Tins::DNS const & pdu);
  auto get(Tins::DNS::QueryType type, DomainName const & name) -> std::vector<std::string>;

  static auto getInstance() -> Cache &;

private:
  Cache(uint32_t seed = std::random_device()()) :
    records_(), generator_(seed)
  {}

  void addFromSection(Tins::DNS::resources_type const & resources, std::chrono::system_clock::time_point const & now);
};


#endif // !CACHE_H
