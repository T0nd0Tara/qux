%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.2"
%define api.parser.class {qux_parser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define parse.error verbose
%locations   // <--

%code requires
{
#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <stack>
#include <cstdint>

enum class id_type {
        undefined, /* undefined */ 
        // function,  /* a pointer to given function */ 
        // parameter, /* one of the function params */ 
        variable,  /* a local variable */
};

struct identifier
{
    id_type type  = id_type::undefined;
    size_t     index = 0; // function#, parameter# within surrounding function, variable#
    std::string     name;
};

enum class ex_type {  
        nop, string, number, ident,       /* atoms */ \
        add, neg, eq,                       /* transformation */ \
        cor, cand, loop,                    /* logic. Loop is: for(param0) { param1..n } */ \
        addrof, deref,                        /* pointer handling */ \
        fcall,                                  /* function param0 call with param1..n */ \
        copy,                                   /* assign: param1 <<- param0 */ \
        comma,                                  /* a sequence of expressions */ \
        ret,                                    /* return(param0) */
};

typedef std::vector<struct expression> expr_vec;
struct expression {
    ex_type type;
    identifier      ident{};    // For ident
    std::string     strvalue{}; // For string
    int32_t    numvalue=0; // For number
    expr_vec        params;
    // For for() and if(), the first item is the condition and the rest are the contingent code
    // For fcall, the first parameter is the variable to use as function

    template<typename... T>
    expression(ex_type t, T&&... args) : type(t), params{ std::forward<T>(args)... } {}

    expression()                    : type(ex_type::nop) {}
    expression(const identifier& i) : type(ex_type::ident),  ident(i)            { }
    expression(identifier&& i)      : type(ex_type::ident),  ident(std::move(i)) { }
    expression(std::string&& s)     : type(ex_type::string), strvalue(std::move(s)) { }
    expression(int32_t v)              : type(ex_type::number), numvalue(v) {}

    bool is_pure() const;

    // TODO: dont know what is this for
    // expression operator%=(expression&& b) && { return expression(ex_type::copy, std::move(b), std::move(*this)); }
};

struct lexctx;
} // %code requires

%param { lexctx& ctx }

%code {
struct lexctx
{
  std::list<std::map<std::string, identifier>> scopes;
public:
  const identifier& define(identifier&& f) {
    auto [it, success] = scopes.begin()->emplace(f.name, std::move(f));
    return it->second;
  }

  expression use(const std::string& name) {
    for (auto it_scope = scopes.begin(); it_scope != scopes.end(); it_scope++) {
      if (auto ident = it_scope->find(name); ident != it_scope->end())
        return ident->second;
    }
  }

  void push_scope() { scopes.emplace_front(); }
  void pop_scope() { scopes.pop_front(); }
};

} // %code

%token END 0
%token RETURN "return" FOR "for" IF "if" ELSE "else" IDENTIFIER NUMLITERAL STRINGLITERAL
%token OR "||" AND "&&" EQ "==" NE "!=" PP "++" MM "--" 
%left ','
%left "||"
%left "&&"
%left "==" "!="
%left '+' '-'
%left '*' '/' '%'
%left '(' '['
%%

library:   declerations;
declerations: declerations decleration
|          %empty;
decleration: IDENTIFIER ':' ':' rvalue ';'
rvalue: function;
function: '(' paramteres ')' stmnt;
stmnt: stmnts
     | decleration ';'
     | "if" expr stmnt
     | "for" expr stmnt
     | "return" expr ';';
stmnts: '{' stmnts1 '}';
stmnts1: stmnt stmnts1
       | %empty;
paramteres: paramteres ',' parameter
          | %empty;
parameter: IDENTIFIER;
expr: NUMLITERAL
    | STRINGLITERAL
    | IDENTIFIER
    | '(' expr ')'
    | IDENTIFIER '(' ')'
    | expr '+' expr
    | expr '-' expr %prec '+'
    | expr '/' expr
    | expr '*' expr %prec '/'
    | expr ',' expr;
%%
