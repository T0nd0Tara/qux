#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <magic_enum.hpp>
#include "lexer.hpp"

const std::string help_txt = \
"Help:\n" \
"-----\n" \
;

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
  std::cout << program << "\n";
  for (auto& token : tokens) {
    std::cout << magic_enum::enum_name(token.type) << ":" << token.value <<"\n";
  }
  return 0;
}
