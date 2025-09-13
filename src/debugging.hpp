#pragma once
#include "textbox.hh"
#include "types.hpp"
#include <string>
#include <sstream>
std::string stringify_type(comp_typing& type) {
  std::stringstream ss;
  ss << magic_enum::enum_name(type.value.type);
  ss << " !> ";
  ss << magic_enum::enum_name(type.error.type);
  return ss.str();
}

struct lexctx;
std::string stringify_types(lexctx &ctx) {
  auto &scope = *ctx.scope.scopes.begin();
  std::stringstream ss;
  std::function<void(Scope&, int)> stringify_scope_types;
  stringify_scope_types = [&](Scope& scope, int depth=0) {

    for(auto [ident_name, ident] : scope.identifiers) {
      ss << std::string(depth * 2, ' ') << ident->name << ": " << stringify_type(ident->type) << '\n';
    }
    for (auto& s : scope.scopes) {
      stringify_scope_types(s, depth+1);
    }
  };
  stringify_scope_types(scope, 0);


  return ss.str();
}
std::string stringify_expr_tree(lexctx &ctx) {
  auto &scope = *ctx.scope.scopes.begin();
  textbox result;
  result.putbox(
      2, 0,
      create_tree_graph(
          scope.expr, 200,
          [&](const expression &e) {
            std::stringstream ss;
            ss << std::string(magic_enum::enum_name(e.type));
            ss << ": ";
            switch (e.type) {
            case ex_type::string: {
              ss << e.strvalue;
              break;
            }
            case ex_type::number: {
              ss << e.numvalue;
              break;
            }
            case ex_type::assign: {
              ss << e.ident->name;
              break;
            }
            case ex_type::func: {
              ss << "(";
              for (size_t i = 0; i < e.params.size(); ++i) {
                ss << e.params[i]->name;
                if (i < e.params.size() - 1)
                  ss << ", ";
              }
              ss << ")";
              break;
            }
            }

            return ss.str();
          },
          [](const expression &e) {
            return std::make_pair(e.children.cbegin(), e.children.cend());
          },
          [](const expression &e) {
            return e.children.size() > 0 || e.params.size() > 0;
          }, // whether simplified horizontal layout can be used
          [](const expression &) {
            return false;
          }, // whether extremely simplified horiz layout can be used
          [](const expression &e) { return e.type == ex_type::loop; }));
  return result.to_string();
}
