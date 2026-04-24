#pragma once
#include <any>
#include <functional>
#include <map>
#include <string>
void qux_print(std::stringstream &, std::any args) {

}
const static std::map<
    std::string, std::function<void(std::stringstream &, std::any args)>>
    c_identifiers{
  std::make_pair("print", qux_print)
};
