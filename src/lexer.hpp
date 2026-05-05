#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <sstream>

enum class TokenType {
  INT_LITERAL,
  STRING_LITERAL,

  OPEN_CURLY_BRACKET,
  CLOSE_CURLY_BRACKET,

  OPEN_PAREN,
  CLOSE_PAREN,

  EQ,
  COLON,
  SEMICOLON,
  COMMA,

  IDENTIFIER,
};

struct Token {
  TokenType type;
  std::string str_val = "";

  size_t row, col;
};

struct LexerError {
  size_t row, col;
  std::string message;
};


std::pair<std::vector<Token>, std::vector<LexerError>> lex_tokens(std::string_view sv);

std::ostream& operator<<(std::ostream& out, const Token& token);
void stringify_tokens(const std::vector<Token> tokens, std::stringstream& ss);

void stringify_lexer_errors(const std::vector<LexerError> errors, std::stringstream& ss);
