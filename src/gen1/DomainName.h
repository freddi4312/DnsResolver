#ifndef DOMAINNAME_H
#define DOMAINNAME_H
// TODO: Change to surely unique symbol.


#include "common.h"
#include <string>
#include <string_view>
#include <vector>


struct DomainName
{
private:
  std::string name_;
  std::vector<int> label_cuts_;

public:
  DomainName(std::string name);
  auto cut(int level) -> std::string_view;
  auto full() -> std::string_view;
  auto labelCount() -> int;
};


#endif // !DOMAINNAME_H
