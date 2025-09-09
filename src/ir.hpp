#pragma once
#include "macros.hpp"
#include "types.hpp"
#include <algorithm>
#include <iterator>
#include <magic_enum/magic_enum.hpp>
#include <sstream>
#include <string>

void generate_function(std::stringstream &ss, const identifier &ident,
                       const expression &func);
void generate_expression(std::stringstream &ss, const expression &e) {
  switch (e.type) {
  case ex_type::string: {
    ss << '"' << e.strvalue << '"';
    break;
  }
  case ex_type::number: {
    ss << e.numvalue;
    break;
  }
  case ex_type::ret: {
    ss << "return ";
    for (const auto &child : e.children)
      generate_expression(ss, child);
    break;
  }
  case ex_type::assign: {
    const expression &rvalue = e.children[0];
    if (rvalue.type == ex_type::func) {
      generate_function(ss, e.ident, rvalue);
      break;
    }

    NOT_IMPLEMENTED("assignment to type: '" +
                    std::string(magic_enum::enum_name(rvalue.type)));
    break;
  }
  case ex_type::fcall: {
    auto name = e.ident.name;

    if (auto c_ident = c_identifiers.find(e.ident.name);
        c_ident != c_identifiers.end()) {
      name = c_ident->second;
    }
    ss << name << "(";
    for (size_t i = 0; i < e.children.size(); ++i) {
      generate_expression(ss, e.children[i]);
      if (i < e.children.size() - 1)
        ss << ", ";
    }
    ss << ")";
    break;
  }
  case ex_type::comp: {
    ss << "{ \n";
    for (const auto &child : e.children) {
      generate_expression(ss, child);
      ss << ";\n";
    }
    ss << "} \n";
    break;
  }

  default:
    NOT_IMPLEMENTED("type: '" + std::string(magic_enum::enum_name(e.type)) +
                    "', cannot be converted to IR");
  }
}

void generate_function(std::stringstream &ss, const identifier &ident,
                       const expression &func) {
  ss << "void " << ident.name << "(";
  for (size_t i = 0; i < func.params.size(); ++i) {
    ss << func.params[i].name;
    if (i != func.params.size())
      ss << ", ";
  }
  ss << ")\n";
  bool need_braces = func.children[0].type != ex_type::comp;
  if (need_braces)
    ss << "{\n";
  generate_expression(ss, func.children[0]);

  if (need_braces)
    ss << "}\n";
}

struct lexctx;
std::string ir_gen(lexctx &ctx) {
  std::stringstream ss;
  ss << R"""(
#include <stdio.h>
)""";
  // The first scope doesnt really store anything now
  auto &scope = *ctx.scope.scopes.begin();
  expression &root = scope.expr;
  for (const expression &e : root.children) {
    generate_expression(ss, e);
  }
  return ss.str();
}
