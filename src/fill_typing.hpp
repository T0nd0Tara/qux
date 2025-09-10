#pragma once
#include "types.hpp"
#include <cassert>
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
comp_typing get_typing_from_assignment(expression &expr) {
  assert(expr.type == ex_type::assign);
  return {
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
