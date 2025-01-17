#pragma once
#include <cctype>
#include <cwctype>
#include <pthread.h>
#include <string_view>
#include <vector>
#include <optional>
#include <cassert>
#include <map>
#include "types.hpp"


namespace  lexer {

const std::map<std::string, TokenType> key_words = {
  {"return", TokenType::return_keyword},
};


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
  std::optional<Token> last_non_typing_token = get_last_non_typing_token(state.tokens);

  const auto next_char = [&]() { return program[state.index + 1]; };
  const auto progress_char = [&]() { return program[++state.index]; };

  char c = program[state.index];
  while (isspace(c)) c = progress_char();

  const std::string_view rest_of_program = program.begin() + state.index;

  for (auto const& [key_word, key_word_token_type] : key_words) {
    if (rest_of_program.starts_with(key_word) && !can_be_in_variable_name(rest_of_program[key_word.size()])) {
      state.index += key_word.size() - 1; // -1 to accomidate the "+1" in the lex_program function
      return Token {
        .type = key_word_token_type,
      };
    }
  }


  if (can_be_in_variable_name(c)) {
    std::string value;
    value += c;

    while (program.length() != state.index + 1 && 
           can_be_in_variable_name(next_char())
    ) {
      value += progress_char();
    }
    return Token {
      .type=TokenType::variable,
      .value=value,
    };
  }


  if (c == '(') return Token{ .type = TokenType::brace_open };
  if (c == ')') return Token{ .type = TokenType::brace_close };

  if (c == '{') return Token{ .type = TokenType::scope_open };
  if (c == '}') return Token{ .type = TokenType::scope_close };

  if (c == ';') return Token{ .type = TokenType::end_statement };

  // We takkle a string litteral
  if (c == '"') {
    Token token;

    token.type = TokenType::string_literal;

    while (true) {
      char last_char = progress_char();
      if (last_char == '"') break;
      token.value += last_char;
    }

    return token;
  }

  if (isdigit(c)) {
    std::string value;
    value += c;

    while (c = next_char(),
      program.length() != state.index + 1 && (isdigit(c) || c == '.')
    ) {
      value += progress_char();
    }
    return Token {
      .type=TokenType::number_literal,
      .value=value,
    };

  }
  
  if (c == ':') {

    if (last_non_typing_token.has_value() && last_non_typing_token.value().type == TokenType::decleration)
      return Token {
        .type = TokenType::comtime_assignment,
      };

    return Token {
      .type = TokenType::decleration,
    };
  }

  return {};
}

std::vector<Token> lex_program(const std::string_view program) {
  State state = {};

  std::optional<Token> token;
  while (token = lex_token(state, program), token.has_value()) {
    state.tokens.push_back(token.value());
    state.index++;
  }

  return state.tokens;
}
}
