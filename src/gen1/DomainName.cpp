#include "DomainName.h"


DomainName::DomainName(std::string name)
  : name_(std::move(name)), label_cuts_()
{
  label_cuts_.push_back(name_.size());

  for (size_t i = name_.size(); i-- > 0;)
  {
    if (name_[i] == '.')
    {
      label_cuts_.push_back(i);
    }
  }
}


auto DomainName::cut(int level) -> std::string_view
{
  const size_t begin = label_cuts_[level];
  const size_t size = name_.size() - begin;

  return std::string_view(&name_[level], size);
}


auto DomainName::full() -> std::string_view
{
  return std::string_view(&name_.front(), name_.size());
}


auto DomainName::labelCount() -> int
{
  return label_cuts_.size();
}
