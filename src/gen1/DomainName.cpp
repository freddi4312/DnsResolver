#include "DomainName.h"
#include <cctype>


DomainName::DomainName(std::string const & name)
  : name_(adapt(name)), label_cuts_()
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


auto DomainName::cut(int level) const -> std::string_view
{
  const size_t begin = label_cuts_[level];
  const size_t size = name_.size() - begin;

  return std::string_view(&name_[level], size);
}


auto DomainName::full() const -> std::string_view
{
  return std::string_view(&name_.front(), name_.size());
}


auto DomainName::labelCount() const -> int
{
  return label_cuts_.size();
}


auto DomainName::adapt(std::string const & name) -> std::string
{
  std::string lower_name;
  std::transform(name.begin(), name.end(), std::back_inserter(lower_name), 
    [](char c) -> char {return std::tolower(c); });

  return lower_name;
}
