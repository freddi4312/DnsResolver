#ifndef DOMAINNAME_H
#define DOMAINNAME_H


#include "common.h"
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <iterator>


struct DomainName
{
private:
  std::string name_;
  std::vector<int> label_cuts_;

public:
  DomainName(std::string const & name);
  auto cut(int level) const -> std::string_view;
  auto full() const -> std::string_view;
  auto labelCount() const -> int;

  static auto adapt(std::string const & name) -> std::string;
};


#endif // !DOMAINNAME_H
