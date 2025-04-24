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
#include "src/types.hpp"

struct lexctx;
} // %code requires

%param { lexctx& ctx }

%code {
struct lexctx
{
  const char* cursor;
  yy::location loc;
  Scope scope;

  Scope& current_scope;

  lexctx(): current_scope(scope) {}
protected:

  identifier* get(const std::string& name) {
    return current_scope.get(name);
  }

public:
  const identifier& define(identifier&& f) {
    return current_scope.define(f);
  }

  expression use(const std::string& name) {
    identifier* ident = get(name);
    if (ident) return *ident;

    throw yy::qux_parser::syntax_error(loc, "Undefined identifier <"+name+">");
  }

  expression call(const std::string& name, const std::vector<expression>& params) {
    identifier* ident = get(name);
    if (ident) return expression(ex_type::fcall, *ident);

    throw yy::qux_parser::syntax_error(loc, "Undefined identifier <"+name+">");
  }

  void push_scope() { 
    current_scope = current_scope.create_child();
  }
  void pop_scope() { 
    if (current_scope.parent == nullptr) throw yy::qux_parser::syntax_error(loc, "Tried to pop root scope");
    current_scope = *current_scope.parent;
  }
};

namespace yy { qux_parser::symbol_type yylex(lexctx& ctx); }

#define M(x) std::move(x)

} // %code

%token END 0
%token RETURN FOR IF ELSE IDENTIFIER NUMLITERAL STRINGLITERAL
%token OR AND EQ NE PP MM
%token PLUS MINUS MULT DIV MOD
%left ','
%left OR
%left AND
%left EQ NE
%left PLUS MINUS
%left MULT DIV MOD
%left '(' '['
%type<int32_t> NUMLITERAL
%type<std::string> IDENTIFIER STRINGLITERAL
%type<expression>  expr
%type<std::vector<expression>>  exprs
%%

library: { ctx.push_scope(); } declerations { ctx.pop_scope(); };
declerations: declerations decleration
|          %empty;
decleration: IDENTIFIER ':' ':' rvalue ';' { ctx.define(identifier{ .name=$1 }); /* currently we only have one type (function) */ }
rvalue: function;
function: { ctx.push_scope(); } '(' parameters ')' stmnt { ctx.pop_scope(); };
stmnt: stmnts
     | decleration ';'
     | expr ';'
     | IF expr stmnt
     | FOR expr stmnt
     | RETURN expr ';';
stmnts: { ctx.push_scope(); } '{' stmnts1 '}' { ctx.pop_scope(); };
stmnts1: stmnt stmnts1
       | %empty;
parameters: parameters ',' parameter 
          | %empty;
parameter: IDENTIFIER 
expr: NUMLITERAL { $$ = $1; }
    | STRINGLITERAL { $$ = M($1); }
    | IDENTIFIER { $$ = ctx.use($1); }
    | '(' expr ')'  { $$ = $2; }
    | IDENTIFIER '(' exprs ')' { $$ = ctx.call($1, $3); }
    | expr PLUS expr
    | expr MINUS expr %prec '+'
    | expr DIV expr
    | expr MULT expr %prec '/'
    | expr ',' expr;
exprs: exprs ',' expr { $$ = M($1); $$.push_back($3); }
     | expr           { $$ = { $1 }; }
     | %empty         { $$ = {}; }
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
    if (!f.is_open()) {
      std::cerr << "Couldn't open file '" << filename << "'. exiting...\n";
      return 1;
    }
    std::string buffer(std::istreambuf_iterator<char>(f), {});
    f.close();

    lexctx ctx;
    ctx.cursor = buffer.c_str();
    ctx.loc.begin.filename = &filename;
    ctx.loc.end.filename   = &filename;

    yy::qux_parser parser(ctx);
    parser.parse();
    // std::vector<function> func_list = std::move(ctx.func_list);

    // for(const auto& f: func_list) std::cout << stringify_tree(f);
}


