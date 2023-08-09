#pragma once


#include <vector>
#include <set>
#include "GlobalCache.h"


struct LocalCache
{
public:
  LocalCache() :
    _globalCache(GlobalCache::getInstance()),
    _A(),
    _NS()
  {}

  LocalCache(LocalCache const & other) = delete;
  LocalCache(LocalCache && other) = delete;

  void addA(std::string const & name, std::string const & ip, uint32_t ttl)
  {
    _A[name] = ip;

    if (ttl > 0 && !_globalCache.hasA(name))
    {
      _globalCache.addA(name, ip, ttl);
    }
  }

  std::string const & getA(std::string const & name)
  {
    if (_A.find(name) == _A.end())
    {
      _A[name] = _globalCache.getA(name);
    }

    return _A.at(name);
  }

  bool hasA(std::string const & name)
  {
    if (_A.find(name) != _A.end() || _globalCache.hasA(name))
    {
      return true;
    }

    return false;
  }

  void addNS(std::string const & node, std::string const & name, uint32_t ttl)
  {
    _NS[node].emplace(name);

    if (ttl > 0 && !_globalCache.hasNS(node))
    {
      _globalCache.addNS(node, name, ttl);
    }
  }

  std::set<std::string> const & getNS(std::string const & node)
  {
    auto It = _NS.find(node);

    if (It == _NS.end() || !It->second.empty())
    {
      _NS[node].emplace(_globalCache.getNS(node));
    }

    return _NS.at(node);
  }

  bool hasNS(std::string const & node)
  {
    auto It = _NS.find(node);

    if ((It != _NS.end() && It->second.empty()) || _globalCache.hasNS(node))
    {
        return true;
    }

    return false;
  }

private:
  GlobalCache &_globalCache;
  std::map<std::string, std::string, NamesCompare> _A;
  std::map<std::string, std::set<std::string>, NamesCompare> _NS;
};