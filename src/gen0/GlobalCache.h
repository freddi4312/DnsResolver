#pragma once


#include <map>
#include <string>
#include <chrono>


struct NamesCompare
{
  bool operator()(std::string const & lhs, std::string const & rhs) const
  {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), 
	                                      rhs.begin(), rhs.end(),
										                    [](char l, char r)
										                    {
                                          return std::tolower(l) < std::tolower(r);
										                    });
  }
};


struct GlobalCache
{
public:
  static GlobalCache & getInstance()
  {
    static GlobalCache instance{};

    return instance;
  }

  GlobalCache(GlobalCache const & other) = delete;
  GlobalCache(GlobalCache && other) = delete;
  ~GlobalCache() = default;

  void addA(std::string const & name, std::string const & ip, uint32_t ttl)
  {
    auto Result = _A.emplace(name, Record(ip, ttl));
    /*
    if (!Result.second)
      Result.first->second.update(ip, ttl);
    */
      
  }

  void addNS(std::string const & node, std::string const & name, uint32_t ttl)
  {
    auto Result = _NS.emplace(node, Record(name, ttl));
    /*
    if (!Result.second)
      Result.first->second.update(name, ttl);
    */
  }

  bool hasA(std::string const & name)
  {
    auto It = _A.find(name);
    return It != _A.end() && !It->second.isObsolete();
  }

  bool hasNS(std::string const &node)
  {
    auto It = _NS.find(node);
    return It != _NS.end() && !It->second.isObsolete();
  }

  std::string getA(std::string const & name)
  {
    return _A.at(name).getData();    
  }

  std::string getNS(std::string const & node)
  {
    return _NS.at(node).getData();
  }

private:
  GlobalCache() = default;

  struct Record
  {
  public:
    Record(std::string const & data, uint32_t ttl) :
      _data(data),
      _ttl(ttl),
      _creationTime(std::chrono::system_clock::now())
    {}

    void update(std::string const & data, uint32_t ttl)
    {
      std::chrono::duration<double> age = std::chrono::system_clock::now() - _creationTime;
      double restOfLife = _ttl - age.count();

      if (restOfLife < ttl)
      {
        _ttl = ttl;
        _creationTime = std::chrono::system_clock::now();
      }
    }

    std::string const & getData() const
    {
        return _data;
    }

    bool isObsolete() const
    {
      std::chrono::duration<double> age = std::chrono::system_clock::now() - _creationTime;
      double seconds = age.count();

      return seconds > _ttl;
    }

  private:
    std::string _data;
    uint32_t _ttl;
    std::chrono::time_point<std::chrono::system_clock> _creationTime;
  };

  std::map<std::string, Record, NamesCompare> _A;
  std::map<std::string, Record, NamesCompare> _NS;
};