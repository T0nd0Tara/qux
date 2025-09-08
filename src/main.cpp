#include "lexer.hpp"
#include "parser/helpers.hpp"
#include "parser/parser.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <sstream>
#include <vector>

const std::string help_txt = "Help:\n"
                             "-----\n";

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << help_txt;
    return 1;
  }
  std::string program_file = argv[1];

  std::ifstream f(program_file);
  if (!f.is_open()) {
    std::cout << "Couldn't open file: " << program_file << ".\n" << help_txt;
    return 1;
  }

  std::string program;
  {
    std::ostringstream ss;
    ss << f.rdbuf();
    f.close();
    program = ss.str();
  }

  std::vector<Token> tokens = lexer::lex_program(program);
  ast::Root root = parser::parse_program(tokens);

  ast::get_tree(std::cout, &root);

  return 0;
}
