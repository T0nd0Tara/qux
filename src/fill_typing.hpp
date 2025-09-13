#pragma once
#include "macros.hpp"
#include "types.hpp"
#include <cassert>
#include <functional>
#include <vector>

expression *find_first_assignment(identifier *ident, expression &expr) {
  if (expr.type == ex_type::assign && expr.ident == ident)
    return &expr;
  for (auto &child : expr.children) {
    expression *child_expr = find_first_assignment(ident, child);
    if (child_expr)
      return child_expr;
  }
  return nullptr;
}
std::vector<expression *>
find_all_expressions_of_type_recursively(expression &expr, ex_type type) {
  std::vector<expression *> out;

  std::function<void(expression &)> rec;
  rec = [&](expression &e) {
    if (e.type == type)
      out.push_back(&e);
    for (expression &child : e.children)
      rec(child);
  };

  rec(expr);
  return out;
}
comp_typing get_typing_from_rvalue(expression &e) {
  switch (e.type) {
  case ex_type::string: {
    return {
        .value = {
          .type = base_type::string,
        }
      };
  }
  case ex_type::number: {
    return {
        .value = {
          .type = base_type::int_,
        }
      };
  }
    default:
      NOT_IMPLEMENTED("get_typing_from_rvalue for expression type: " + std::string(magic_enum::enum_name(e.type)));
  }
}
std::vector<typing> get_input_type_of_func(expression &expr) {
  return {};
}
comp_typing get_output_type_of_func(expression &expr) {
  expression &func_body = expr.children.front();
  if (func_body.type == ex_type::comp) {
    std::vector<expression *> returns =
        find_all_expressions_of_type_recursively(expr, ex_type::ret);
    if (returns.size() == 1) {
      expression& ret_rvalue = returns.front()->children.front();
      return get_typing_from_rvalue(ret_rvalue);
    }
      

    comp_typing out = {.value = {.type = base_type::or_},
                       .error = {.type = base_type::or_}};
    for (expression *ret_expr : returns) {
      expression& ret_rvalue = ret_expr->children.front();
      comp_typing ret_type = get_typing_from_rvalue(ret_rvalue);
      out.value.children.push_back(ret_type.value);
      out.error.children.push_back(ret_type.error);
    }
    return out;
  }

  NOT_IMPLEMENTED("get_output_type_of_func: func_body.type != ex_type::comp");
}
comp_typing get_type_of_func(expression &expr) {
  comp_typing out{.value = {.type = base_type::func},
                  .error = {.type = base_type::void_}};
  out.value.children = get_input_type_of_func(expr);
  auto out_type = get_output_type_of_func(expr);
  out.value.func_output = std::make_shared<comp_typing>(std::move(out_type));
  return out;
}
comp_typing get_typing_from_assignment(expression &expr) {
  assert(expr.type == ex_type::assign);

  auto &rvalue = expr.children[0];
  switch (rvalue.type) {
  case ex_type::func: {
    return get_type_of_func(rvalue);
  }
  default:
    NOT_IMPLEMENTED("type: '" +
                    std::string(magic_enum::enum_name(rvalue.type)) +
                    "', cannot be assigned");
  }
  return comp_typing{
      .value = {.type = base_type::void_},
  };
}
void fill_scope_typing(Scope &scope) {
  // we map the implicitly typed identifiers to where in the code they are first
  // assigned to a value
  std::vector<identifier *> untyped_idents;
  for (auto it = scope.identifiers.begin(); it != scope.identifiers.end();
       ++it) {
    if (!it->second->explicit_typing)
      untyped_idents.push_back(it->second);
  }

  // TODO: split the work between threads,
  // Each one can check a different identifiers assignment

  for (auto it = untyped_idents.begin(); it != untyped_idents.end(); ++it) {
    expression *first_assignment = find_first_assignment(*it, scope.expr);
    if (first_assignment) {
      (*it)->type = get_typing_from_assignment(*first_assignment);
    }
  }
}
struct lexctx;
void fill_typing(lexctx &ctx) {
  auto &scope = *ctx.scope.scopes.begin();
  fill_scope_typing(scope);
}
