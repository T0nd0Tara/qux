#pragma once
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

// enum class id_type {
//         undefined, /* undefined */
//         // function,  /* a pointer to given function */
//         // parameter, /* one of the function params */
//         variable,  /* a local variable */
// };
//
struct identifier {
  // id_type type  = id_type::undefined;
  // size_t     index = 0; // function#, parameter# within surrounding function,
  // variable#
  std::string name;
};

enum class ex_type {
  nop,
  string,
  number,
  ident, /* atoms */
  add,
  neg,
  eq, /* transformation */
  cor,
  cand,
  loop, /* logic. Loop is: for(param0) { param1..n } */
  addrof,
  deref, /* pointer handling */
  fcall, /* function param0 call with param1..n */
  copy,  /* assign: param1 <<- param0 */
  comma, /* a sequence of expressions */
  ret,   /* return(param0) */
};

typedef std::vector<struct expression> expr_vec;
struct expression {
  ex_type type;
  identifier ident{};     // For ident
  std::string strvalue{}; // For string
  int32_t numvalue = 0;   // For number
  expr_vec params;
  // For for() and if(), the first item is the condition and the rest are the
  // contingent code For fcall, the first parameter is the variable to use as
  // function

  template <typename... T>
  expression(ex_type t, T &&...args)
      : type(t), params{std::forward<T>(args)...} {}

  expression() : type(ex_type::nop) {}
  expression(const identifier &i) : type(ex_type::ident), ident(i) {}
  expression(identifier &&i) : type(ex_type::ident), ident(std::move(i)) {}
  expression(std::string &&s) : type(ex_type::string), strvalue(std::move(s)) {}
  expression(int32_t v) : type(ex_type::number), numvalue(v) {}

  bool is_pure() const;

  // TODO: dont know what is this for
  // expression operator%=(expression&& b) && { return expression(ex_type::copy,
  // std::move(b), std::move(*this)); }
};
struct Statement {};

struct Scope {
  std::map<std::string, Statement> statements;
  std::list<Scope> scopes;

  Scope *parent = nullptr;

  Scope &create_child() {
    Scope &child = scopes.emplace_back();

    child.parent = this;
    return child;
  }
};
