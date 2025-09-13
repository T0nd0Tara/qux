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

  lexctx(): current_scope(&scope) {
    for (const auto [qux_name, c_name] : c_identifiers) {
      define(new identifier{.name = qux_name});
    }
  }
protected:

  identifier* get(const std::string& name) {
    return current_scope->get(name);
  }

public:
  identifier* define(identifier* f) {
    return current_scope->define(f);
  }

  identifier* get_identifier(const std::string& name) {
    identifier* ident = get(name);
    if (ident) return ident;

    throw yy::qux_parser::syntax_error(loc, "Undefined identifier <"+name+">");
  }

  expression call(const std::string& name, const expr_vec& children) {
    identifier* ident = get_identifier(name);
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
%type<identifier*> parameter 
%type<std::vector<identifier*>>  parameters

/* Generate the parser description file. */
%verbose
/* Enable run-time traces (yydebug). */
%define parse.trace
%%

library: { ctx.push_scope(); } declerations {  ctx.current_scope->expr = M($2); ctx.pop_scope(); };
declerations: declerations decleration { $$ = M($1); $$.children.push_back(M($2)); }
            | %empty                   { $$ = expression(ex_type::comp); };
decleration: IDENTIFIER ':' ':' rvalue ';' { $$ = expression(ex_type::assign); $$.ident = ctx.define(new identifier{ .name=$1 }); $$.children.push_back($4); /* currently we only have one type (function) */ }
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
parameter: IDENTIFIER { $$ = ctx.define(new identifier{ .name = M($1) }); }
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
#include <boost/program_options.hpp>

void yy::qux_parser::error(const location_type& l, const std::string& m)
{
    std::cerr << (l.begin.filename ? l.begin.filename->c_str() : "(undefined)");
    std::cerr << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << m << '\n';
}



#include "src/ir.hpp"
#include "src/fill_typing.hpp"
#include "src/debugging.hpp"

int read_program(std::string filename, std::string& buffer) {
    std::ifstream f(filename);
    if (!f.is_open()) {
      return 1;
    }
    buffer = std::string(std::istreambuf_iterator<char>(f), {});
    f.close();
    return 0;
}
int write_ir(std::string filename, std::string_view ir) {
    std::ofstream f(filename, std::ios::trunc);
    if (!f.is_open()) {
      return 1;
    }
    f << ir;
    f.close();
    return 0;
}

int cli_handle(int argc, char** argv) {
  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("output,o", po::value<std::string>(), "output file")
    ("filename", po::value<std::string>(), "input file")
    ("print-ast", po::bool_switch(), "prints the AST to the console")
    ("print-types", po::bool_switch(), "prints the varibales types to the console")
    ("print-ir", po::bool_switch(), "prints the intermediate representation to the console")
  ;
  po::positional_options_description pos;
  pos.add("filename", 1);  // first positional arg is filename

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
    .options(desc)
    .positional(pos)
    .run()
    , vm);
  po::notify(vm);    

  if (vm.count("help")) {
      std::cout << "Usage: " << argv[0] << " [options] filename\n";
      std::cout << desc << "\n";
      return 0;
  }
  if (vm.count("filename") == 0) {
      std::cerr << "Pleas provide an input file\n";
      return 1;
  }

  std::string input_file = vm["filename"].as<std::string>();
  std::string input_file_without_extention = input_file.substr(0, input_file.find_last_of("."));
  std::string output_file = input_file_without_extention + ".c";

  if (vm.count("output")) {
    output_file = vm["output"].as<std::string>();
  }
  int ret;
  std::string buffer;
  ret = read_program(input_file, buffer);
  if (ret) {
    std::cerr << "Couldn't read file '" << input_file << "'. exiting...\n";
    return ret;
  }
  
  lexctx ctx;
  ctx.cursor = buffer.c_str();
  ctx.loc.begin.filename = &input_file;
  ctx.loc.end.filename   = &input_file;

  yy::qux_parser parser(ctx);
  // parser.set_debug_level(1);
  ret = parser.parse();
  if (ret) {
    std::cerr << "Parsing Error\n";
    return ret;
  }
  fill_typing(ctx);

  if (vm["print-ast"].as<bool>()) {
    std::cout << stringify_expr_tree(ctx);
    std::cout << "\n\n";
  }
  if (vm["print-types"].as<bool>()) {
    std::cout << stringify_types(ctx);
    std::cout << "\n\n";
  }
  std::string ir = ir_gen(ctx);
  if (vm["print-ir"].as<bool>()) {
    std::cout << ir;
    std::cout << "\n\n";
  }
  ret = write_ir(output_file, ir);
  if (ret) std::cerr << "Couldn't write to file '" << output_file << "'. exiting...\n";
  return ret;
}
int main(int argc, char** argv)
{
  try {
    return cli_handle(argc, argv);
  }
  catch(std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
  }
  catch(...) {
    std::cerr << "Exception of unknown type!\n";
  }

  return 1;

}


