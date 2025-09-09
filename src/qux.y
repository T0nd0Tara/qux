%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.2"
%define api.parser.class {qux_parser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define parse.error verbose
%locations   // <--

%code requires {
#include <magic_enum/magic_enum.hpp>
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

  Scope* current_scope;

  lexctx(): current_scope(&scope) {}
protected:

  identifier* get(const std::string& name) {
    return current_scope->get(name);
  }

public:
  const identifier& define(identifier&& f) {
    return current_scope->define(f);
  }

  identifier& get_identifier(const std::string& name) {
    identifier* ident = get(name);
    if (ident) return *ident;

    throw yy::qux_parser::syntax_error(loc, "Undefined identifier <"+name+">");
  }

  expression call(const std::string& name, const expr_vec& children) {
    identifier& ident = get_identifier(name);
    expression expr = expression(ex_type::fcall, children);
    expr.ident = ident;
    return expr;
  }

  void push_scope() { 
    current_scope = current_scope->create_child();
  }
  void pop_scope() { 
    if (current_scope->parent == nullptr) throw yy::qux_parser::syntax_error(loc, "Tried to pop root scope");
    current_scope = current_scope->parent;
  }
};

namespace yy { qux_parser::symbol_type yylex(lexctx& ctx); }

#define M(x) std::move(x)

} // %code

%token END 0
%token RETURN FOR IF ELSE IDENTIFIER NUMLITERAL STRINGLITERAL COMP
%token OR AND EQ NE PP MM
%token PLUS MINUS MULT DIV MOD
%left ','
%left OR
%left AND
%left EQ NE
%left PLUS MINUS
%left MULT DIV MOD
%left COMP
%left '(' '['
%type<int32_t> NUMLITERAL
%type<std::string> IDENTIFIER STRINGLITERAL
// TODO: currently there is no difference between decleration, statement and expression, should there be?
%type<expression>  expr declerations decleration stmnt stmnts stmnts1 rvalue function 
%type<std::vector<expression>>  exprs
%type<identifier> parameter 
%type<std::vector<identifier>>  parameters

/* Generate the parser description file. */
%verbose
/* Enable run-time traces (yydebug). */
%define parse.trace
%%

library: { ctx.push_scope(); } declerations {  ctx.pop_scope(); ctx.current_scope->expr = M($2); };
declerations: declerations decleration { $$ = M($1); $$.children.push_back(M($2)); }
            | %empty                   { $$ = expression(ex_type::comp); };
decleration: IDENTIFIER ':' ':' rvalue ';' { $$ = expression(ex_type::assign); $$.ident = ctx.define(identifier{ .name=$1 }); $$.children.push_back($4); /* currently we only have one type (function) */ }
rvalue: function { $$ = M($1); };
function: { ctx.push_scope(); } '(' parameters ')' stmnt { $$ = expression(ex_type::func, $5); $$.params = $3; ctx.pop_scope(); };
stmnt: stmnts           { $$ = M($1); }
     | decleration ';'  { $$ = M($1); }
     | expr ';'         { $$ = M($1); }
     | IF expr stmnt    { $$ = expression(ex_type::cond, $2, $3); }
     | FOR expr stmnt   { $$ = expression(ex_type::loop, $2, $3); }
     | RETURN expr ';'  { $$ = expression(ex_type::ret, $2); };
stmnts: { ctx.push_scope(); } '{' stmnts1 '}' { $$ = M($3); ctx.pop_scope(); };
stmnts1: stmnts1 stmnt { $$ = M($1); $$.children.push_back(M($2)); }
       | %empty { $$ = expression(ex_type::comp); };
parameters: parameters ',' parameter { $$ = M($1); $$.push_back($3); }
          | %empty { $$ = ident_vec(); }
parameter: IDENTIFIER { $$ = ctx.define(identifier{ .name = M($1) }); }
expr: NUMLITERAL { $$ = $1; }
    | STRINGLITERAL { $$ = M($1); }
    | '(' expr ')' %prec COMP { $$ = $2; }
    | IDENTIFIER '(' exprs ')'   { $$ = ctx.call($1, $3); }
    | expr PLUS expr  %prec PLUS { $$ = expression(ex_type::plus, $1, $3); }
    | expr MINUS expr %prec PLUS { $$ = expression(ex_type::minus, $1, $3); }
    | expr DIV expr   %prec DIV  { $$ = expression(ex_type::div, $1, $3); }
    | expr MULT expr  %prec MULT { $$ = expression(ex_type::mult, $1, $3); };
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

#include <sstream>
#include <fstream>
#include "textbox.hh"
void yy::qux_parser::error(const location_type& l, const std::string& m)
{
    std::cerr << (l.begin.filename ? l.begin.filename->c_str() : "(undefined)");
    std::cerr << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << m << '\n';
}


std::string stringify_tree(lexctx& ctx) {
    textbox result;
      result.putbox(2,0, create_tree_graph(ctx.scope.expr, 200,
          [&](const expression& e)
          {
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
              ss << e.ident.name; 
              break;
            }
            case ex_type::func: {
              ss << "("; 
              for (int i = 0; i < e.params.size(); ++i) {
                ss << e.params[i].name;
                if (i != e.params.size())
                  ss << ", ";
              }
              ss << ")"; 
              break;
            }
            }

            return ss.str();
          },
          [](const expression& e) { return std::make_pair(e.children.cbegin(), e.children.cend()); },
          [](const expression& e) { return e.children.size() > 0 || e.params.size() > 0; }, // whether simplified horizontal layout can be used
          [](const expression&  ) { return false; },                 // whether extremely simplified horiz layout can be used
          [](const expression& e) { return e.type == ex_type::loop; }));
    return result.to_string();
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
    // parser.set_debug_level(1);
    int parse_ret = parser.parse();
    if (parse_ret) {
      std::cerr << "Parsing Error\n";
      return parse_ret;
    }

    std::cout << stringify_tree(ctx);
    // std::vector<function> func_list = std::move(ctx.func_list);

    // for(const auto& f: func_list) std::cout << stringify_tree(f);
}


