#pragma once
#include <any>
#include <cassert>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class base_type {
  void_, // if 'nil' means empty, void means nothing. for example what a
         // compound statement returns
  nil,
  string,
  int_,
  array, // for example the type '[string, int]', has base_type of array with
         // children string, int
  or_,   // ex: 'string | int'
  func,
  // func_input, // we don't need a func_output as the output is one type, not
  //             // array of different types
  //             // basically this type is just a container to put the types
  //             array
  //             // in the children
};
struct comp_typing;

struct typing {
  base_type type;
  std::vector<typing> children{};
  bool array = false;

  std::shared_ptr<comp_typing> func_output;
};
struct comp_typing {
  typing value;
  typing error{.type = base_type::void_};
};
struct identifier {
  std::string name;

  bool explicit_typing = false;
  comp_typing type{};
};
typedef std::vector<identifier *> ident_vec;

enum class ex_type {
  nop,
  string,
  int_,
  // ident, /* atoms */
  assign,
  plus,
  minus,
  mult,
  div,
  neg,
  eq, /* transformation */
  cor,
  cand,
  comp, // compound statement
        // (i.e. a statement that just contain multiple statements)
  cond,
  loop, /* logic. Loop is: for(param0) { param1..n } */
  addrof,
  deref, /* pointer handling */
  fcall, /* function param0 call with param1..n */
  func,
  copy,  /* assign: param1 <<- param0 */
  comma, /* a sequence of expressions */
  ret,   /* return(param0) */
};

typedef std::vector<struct expression> expr_vec;
struct expression {
  ex_type type;
  identifier *ident = nullptr;
  std::string strvalue{}; // For string
  int32_t numvalue = 0;   // For number
  //
  // For for() and if(), the first item is the condition and the rest are the
  // contingent code For fcall, the first parameter is the variable to use as
  // function
  expr_vec children{};

  ident_vec params{};

  template <typename... T>
  expression(ex_type t, T &&...args)
      : type(t), children{std::forward<T>(args)...} {}

  expression(ex_type t, expr_vec &&args) : type(t), children(std::move(args)) {}

  expression() : type(ex_type::nop) {}
  // expression(const identifier &i) : type(ex_type::ident), ident(i) {}
  // expression(identifier &&i) : type(ex_type::ident), ident(std::move(i)) {}
  expression(std::string &&s) : type(ex_type::string), strvalue(std::move(s)) {}
  expression(int32_t v) : type(ex_type::int_), numvalue(v) {}

  bool is_pure() const;

  // TODO: dont know what is this for
  // expression operator%=(expression&& b) && { return expression(ex_type::copy,
  // std::move(b), std::move(*this)); }
};
struct Statement {};

struct Scope {
  expression expr;
  std::map<std::string, identifier *> identifiers;
  std::list<Scope> scopes;

  Scope *parent = nullptr;

  Scope(Scope *parent_scope = nullptr) : parent(parent_scope) {}

  ~Scope() {
    for (auto it = identifiers.begin(); it != identifiers.end(); ++it) {
      delete it->second;
    }
  }

  Scope *create_child() { return &scopes.emplace_back(this); }

  identifier *get(std::string name) {
    if (auto search = identifiers.find(name); search != identifiers.end())
      return search->second;
    if (parent)
      return parent->get(name);
    return nullptr;
  }
  identifier *define(identifier *f) {
    auto [it, success] = identifiers.insert({f->name, f});
    return it->second;
  }
};
