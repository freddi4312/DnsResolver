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
#include "Record.h"


class Cache
{
private:
  std::unordered_map<RecordName, std::list<Record>> records_;
  std::mt19937 generator_;

public:
  Cache(Cache const &) = delete;
  void add(Tins::DNS const & pdu);
  auto get(Tins::DNS::QueryType type, DomainName const & name) -> std::vector<std::string>;
  void addRootServers(std::vector<std::pair<std::string, std::string>> const & servers);

  static auto getInstance() -> Cache &;

private:
  Cache(uint32_t seed = std::random_device()()) :
    records_(), generator_(seed)
  {}

  void addFromSection(Tins::DNS::resources_type const & resources, std::chrono::system_clock::time_point const & now);
};


#endif // !CACHE_H
