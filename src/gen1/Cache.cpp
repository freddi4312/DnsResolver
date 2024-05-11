#include "Cache.h"
#include <algorithm>


//using clock = std::chrono::system_clock;
using namespace std::literals::chrono_literals;


auto Cache::getInstance() -> Cache &
{
  uint32_t const magic_number = 13; // just seed
  static Cache instance = Cache(magic_number);

  return instance;
}


void Cache::add(Tins::DNS const & pdu)
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

  addFromSection(pdu.authority(), now);
  addFromSection(pdu.answers(), now);
  addFromSection(pdu.additional(), now);
}


void Cache::addFromSection(Tins::DNS::resources_type const & resources, std::chrono::system_clock::time_point const & now)
{
  for (Tins::DNS::resource const & rsc : resources)
  {
    if (rsc.query_type() != Tins::DNS::A 
      && rsc.query_type() != Tins::DNS::NS
      && rsc.query_type() != Tins::DNS::CNAME)
    {
      continue;
    }

    std::chrono::system_clock::time_point valid_until = now + 1s * rsc.ttl();

    RecordName record_name
    (
      static_cast<Tins::DNS::QueryType>(rsc.query_type()),
      adapt(rsc.dname())
    );

    auto & record_list = records_[record_name];
    bool is_time_updated = false;

    for (auto & record : record_list)
    {
      if (isSameName(record.data, rsc.data()))
      {
        record.valid_until = std::max(valid_until, record.valid_until);
        is_time_updated = true;
        break;
      }
    }

    if (!is_time_updated)
    {
      Record record(adapt(rsc.data()), valid_until);
      record_list.push_back(record);
    }
  }
}


auto Cache::get(Tins::DNS::QueryType type, DomainName const & name) -> std::vector<std::string>
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::vector<std::string> result;

  int label = name.labelCount();
  do
  {
    label--;
    RecordName record_name(type, std::string(name.cut(label)));
    auto record_it = records_.find(record_name);

    if (record_it == records_.end())
    {
      continue;
    }

    auto record_list = record_it->second;
    result.reserve(record_it->second.size());

    for (auto list_it = record_list.begin(); list_it != record_list.end(); )
    {
      if (list_it->valid_until < now)
      {
        list_it = record_list.erase(list_it);
      }
      else
      {
        result.push_back(list_it->data);
        ++list_it;
      }
    }

  } while (type == Tins::DNS::NS && label > 0 && result.empty());
  // looping to root for a NS query type

  // random shuffling root name servsers list
  if (label == 0)
  {
    std::shuffle(result.begin(), result.end(), generator_);
  }

  return result;
}


void Cache::addRootServers(std::vector<std::pair<std::string, std::string>> const & servers)
{
  std::chrono::system_clock::time_point valid_until = std::chrono::system_clock::time_point::max();

  for (auto const & server : servers)
  {
    records_[{Tins::DNS::NS, ""}].emplace_back(server.first, valid_until);
    records_[{Tins::DNS::A, server.first}].emplace_back(server.second, valid_until);
  }
}
