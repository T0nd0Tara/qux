#pragma once
#include <cwctype>
#include <pthread.h>
#include <string_view>
#include <vector>
#include <optional>
#include <iostream>
#include "magic_enum.hpp"
#include "types.hpp"


namespace  lexer {
struct State {
  size_t index = 0;
  std::vector<Token> tokens = {};
};

inline bool can_be_in_variable_name(char c) {
  return isalpha(c) || c == '_';
}

inline std::optional<Token> get_last_non_typing_token(const std::vector<Token>& tokens) {
  for (size_t i = tokens.size() - 1; i < tokens.size(); i--) {
    if (tokens[i].type != TokenType::typing) return tokens[i];
  }
  return {};
}

std::optional<Token> lex_token(State& state, const std::string_view program) {
  Token token;
  std::optional<Token> last_non_typing_token = get_last_non_typing_token(state.tokens);

  for (; state.index < program.length(); state.index ++) {
    const char c = program[state.index];
    if (can_be_in_variable_name(c)) {
      token.value += c;
      continue;
    }

    if (c == ':') {
      if (token.value.length() > 0) {
        token.type = TokenType::variable;
        return token;
      }
      state.index++;

      if (last_non_typing_token.has_value() && last_non_typing_token.value().type == TokenType::decleration)
        return Token {
          .type = TokenType::comtime_assignment,
        };

      return Token {
        .type = TokenType::decleration,
      };
    }

    if (isspace(c)) {
      return {};
    }
  }
  return {};
}

std::vector<Token> lex_program(const std::string_view program) {
  State state = {};

  std::optional<Token> token;
  while (token = lex_token(state, program), token.has_value()) {
    state.tokens.push_back(token.value());
  }

  return state.tokens;
}
}
