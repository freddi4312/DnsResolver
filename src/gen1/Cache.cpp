#include "Cache.h"
#include <algorithm>


using clock = std::chrono::system_clock;
using namespace std::literals::chrono_literals;


auto Cache::getInstance() -> Cache &
{
  uint32_t const magic_number = 13; // just seed
  static Cache instance = Cache(magic_number);

  return instance;
}


void Cache::add(Tins::DNS const & pdu)
{
  clock::time_point now = clock::now();

  addFromSection(pdu.authority(), now);
  addFromSection(pdu.answers(), now);
  addFromSection(pdu.additional(), now);
}


void Cache::addFromSection(Tins::DNS::resources_type const & resources, clock::time_point const & now)
{
  for (Tins::DNS::resource const & rsc : resources)
  {
    clock::time_point valid_until = now + 1s * rsc.ttl();

    RecordName record_name
    (
      static_cast<Tins::DNS::QueryType>(rsc.query_type()),
      std::string(rsc.dname())
    );

    auto record_list = records_[record_name];
    bool is_time_updated = false;

    for (auto it = record_list.begin(); it != record_list.end(); )
    {
      if (it->data == rsc.data())
      {
        it->valid_until = std::max(valid_until, it->valid_until);
        is_time_updated = true;
        break;
      }
    }

    if (!is_time_updated)
    {
      Record record(rsc.data(), valid_until);
      record_list.push_back(record);
    }
  }
}


auto Cache::get(Tins::DNS::QueryType type, DomainName const & name) -> std::vector<std::string>
{
  clock::time_point now = clock::now();
  std::vector<std::string> result;

  int label = 0;
  do
  {
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

    label++;
  } while (type == Tins::DNS::QueryType::NS && label < name.labelCount() && result.empty());
  // looping to root for a NS query type

  // random shuffling root name servsers list
  if (label == name.labelCount())
  {
    std::shuffle(result.begin(), result.end(), generator_);
  }

  return result;
}
