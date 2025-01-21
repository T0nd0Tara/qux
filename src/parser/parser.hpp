#pragma once
#include <iterator>
#include <sstream>
#include <cassert>
#include <magic_enum.hpp>
#include <iostream>
#include "../lexer.hpp"
#include "../types.hpp"
#include "ast_types.hpp"
#include "helpers.hpp"
#include "../macros.hpp"

namespace parser {

void parse_section(const std::vector<Token>& tokens, ast::Nodes& out, size_t& i) {
  const auto next_token = [&]() { return tokens[i + 1]; };
  const auto prev_token = [&]() { return tokens[i - 1]; };
  const auto progress_token = [&]() { return tokens[++i]; };

  while (i < tokens.size()) {
    Token token = tokens[i];

    switch (token.type) {
      case TokenType::decleration: {
        if (prev_token().type != TokenType::variable) {
          std::cerr << "Declering after token " << magic_enum::enum_name((prev_token().type);
          assert(false);
        }

        if (next_token().type == TokenType::typing) {
          NOT_IMPLEMENTED("Typing after decleration");
        }

        if (next_token().type != TokenType::comtime_assignment
         && next_token().type != TokenType::runtime_assignment
        ) {
          std::cerr << "Decleration found without assignment. found token: " << magic_enum::enum_name(prev_token().type);
          assert(false);
        }


        out.push_back(ast::Decleration)
        break;
      }
      case TokenType::variable: {
        if (next_token().type != TokenType::decleration) {
          assert(false && "UNHANDLED: variable used, not declared");
          break;
        }


        break;
      }
      default:
        std::cerr << "Cant handle token: " << magic_enum::enum_name(token.type) << '\n';
        assert(false && "Cant handle token");
    }

    progress_token();
  }


}

ast::Root parse_program(const std::vector<Token>& tokens) {
  ast::Root root;

  size_t i = 0;

  parse_section(tokens, root.nodes, i);

  return std::move(root);
}
}
