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

// enum class id_type {
//         undefined, /* undefined */ 
//         // function,  /* a pointer to given function */ 
//         // parameter, /* one of the function params */ 
//         variable,  /* a local variable */
// };
//
struct identifier
{
    // id_type type  = id_type::undefined;
    // size_t     index = 0; // function#, parameter# within surrounding function, variable#
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
    const char* cursor;
    yy::location loc;
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

    throw yy::qux_parser::syntax_error(loc, "Undefined identifier <"+name+">");
  }

  void push_scope() { scopes.emplace_front(); }
  void pop_scope() { scopes.pop_front(); }
};

namespace yy { qux_parser::symbol_type yylex(lexctx& ctx); }

#define M(x) std::move(x)

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
%type<int32_t> NUMLITERAL
%type<std::string> IDENTIFIER STRINGLITERAL
%type<expression>  expr
%%

library: { ctx.push_scope(); } declerations { ctx.pop_scope(); };
declerations: declerations decleration
|          %empty;
decleration: IDENTIFIER ':' ':' rvalue ';' { ctx.define(identifier{ .name=$1 }); /* currently we only have one type (function) */ }
rvalue: function;
function: '(' parameters ')' stmnt;
stmnt: stmnts
     | decleration ';'
     | "if" expr stmnt
     | "for" expr stmnt
     | "return" expr ';';
stmnts: '{' stmnts1 '}';
stmnts1: stmnt stmnts1
       | %empty;
parameters: parameters ',' parameter 
          | %empty;
parameter: IDENTIFIER 
expr: NUMLITERAL { $$ = $1; }
    | STRINGLITERAL { $$ = M($1); }
    | IDENTIFIER { $$ = ctx.use($1); }
    | '(' expr ')'  { $$ = $2; }
    // | IDENTIFIER '(' ')'
    | expr '+' expr
    | expr '-' expr %prec '+'
    | expr '/' expr
    | expr '*' expr %prec '/'
    | expr ',' expr;
%%
yy::qux_parser::symbol_type yy::yylex(lexctx& ctx)
{
    const char* anchor = ctx.cursor;
    ctx.loc.step();
    auto s = [&](auto func, auto&&... params) 
    { 
      ctx.loc.columns(ctx.cursor - anchor);
      return func(params..., ctx.loc);
    };

%{ /* Begin re2c lexer */
re2c:yyfill:enable   = 0;
re2c:define:YYCTYPE  = "char";
re2c:define:YYCURSOR = "ctx.cursor";

// Keywords:
"return"                { return s(qux_parser::make_RETURN); }
"for"                   { return s(qux_parser::make_FOR); }
"if"                    { return s(qux_parser::make_IF); }

// Identifiers:
[a-zA-Z_] [a-zA-Z_0-9]* { return s(qux_parser::make_IDENTIFIER, std::string(anchor,ctx.cursor)); }

// String and integer literals:
"\"" [^"]* "\""         { return s(qux_parser::make_STRINGLITERAL, std::string(anchor+1, ctx.cursor-1)); }
[0-9]+                  { return s(qux_parser::make_NUMLITERAL, std::stol(std::string(anchor,ctx.cursor))); }

// Whitespace and comments:
"\000"                  { return s(qux_parser::make_END); }
"\r\n" | [\r\n]         { ctx.loc.lines();   return yylex(ctx); }
"//" [^\r\n]*           {                    return yylex(ctx); }
[\t\v\b\f ]             { ctx.loc.columns(); return yylex(ctx); }

// Multi-char operators and any other character (either an operator or an invalid symbol):
"&&"                    { return s(qux_parser::make_AND); }
"||"                    { return s(qux_parser::make_OR); }
"++"                    { return s(qux_parser::make_PP); }
"--"                    { return s(qux_parser::make_MM); }
"!="                    { return s(qux_parser::make_NE); }
"=="                    { return s(qux_parser::make_EQ); }
.                       { return s([](auto...s){return qux_parser::symbol_type(s...);}, qux_parser::token_type(ctx.cursor[-1]&0xFF)); } // Return that character
%} /* End lexer */
}

#include <fstream>
void yy::qux_parser::error(const location_type& l, const std::string& m)
{
    std::cerr << (l.begin.filename ? l.begin.filename->c_str() : "(undefined)");
    std::cerr << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << m << '\n';
}

int main(int argc, char** argv)
{
    if (argc < 2) {
      std::cerr << "Input file must be given\n";
      return 1;
    }
    std::string filename = argv[1];
    std::ifstream f(filename);
    std::string buffer(std::istreambuf_iterator<char>(f), {});

    lexctx ctx;
    ctx.cursor = buffer.c_str();
    ctx.loc.begin.filename = &filename;
    ctx.loc.end.filename   = &filename;

    yy::qux_parser parser(ctx);
    parser.parse();
    // std::vector<function> func_list = std::move(ctx.func_list);

    // for(const auto& f: func_list) std::cerr << stringify_tree(f);
}


