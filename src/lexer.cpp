#include "./lexer.hpp"
#include <cctype>
#include <sstream>
#include <magic_enum/magic_enum.hpp>
#include <iomanip>
#include <cmath>
#include <unordered_map>

static const std::unordered_map<char, TokenType> simple_tokens = {
  { '{', TokenType::OPEN_CURLY_BRACKET },
  { '}', TokenType::CLOSE_CURLY_BRACKET },


  { '(', TokenType::OPEN_PAREN },
  { ')', TokenType::CLOSE_PAREN },

  { '=', TokenType::EQ },
  { ':', TokenType::COLON },

  { ';', TokenType::SEMICOLON },

  { ',', TokenType::COMMA },
};

bool is_identifier_char(char c) {
  // It's ok for numbers to be present, as if the number is in the first position,
  // it would go into the number literal flow
  return std::isdigit(c) || std::isalpha(c);
}


std::pair<std::vector<Token>, std::vector<LexerError>> lex_tokens(std::string_view sv) {
  std::vector<Token> out;
  std::vector<LexerError> errors;

  size_t col = 0;
  size_t row = 0;

  size_t i = 0;
  const auto inc_index = [&] () {
    const char c = sv[i];
    
    if (c == '\n') {
      col = 0; 
      row++; 
    } else {
      col++;
    }

    return ++i;
  };
  while (i < sv.length()) {
    const char c = sv[i];
    // TODO: Handle escapes inside string, like "\""
    if (c == '"') {
      size_t start_row = row;
      size_t start_col = col;
      std::stringstream current_string;
      if (i + 1 == sv.length()) {
        errors.push_back(LexerError{
          .row = row,
          .col = col,
          .message = "String Literal Can't End The File",
        });
        break;
      }

      char it_c;
      while (it_c = sv[inc_index()], it_c != '"') {
        current_string << it_c;
        if (i + 1 == sv.length()) {
          errors.push_back(LexerError{
            .row = row,
            .col = col,
            .message = "Unfinished String Literal",
          });
          break;
        }
      }
      out.push_back(Token{
        .type = TokenType::STRING_LITERAL,
        .str_val = current_string.str(),
        .row = start_row,
        .col = start_col,
      });

      goto CONTINUE;
    }

    if (std::isdigit(c)) {
      size_t start_row = row;
      size_t start_col = col;
      std::stringstream ss;
      char it_c = c;
      for (;;) {
        ss << it_c;
        if (sv.length() == i + 1) break;
        if (!std::isdigit(sv[i + 1])) break;
        it_c = sv[inc_index()];
      }

      out.push_back(Token{
        .type = TokenType::INT_LITERAL,
        .str_val = ss.str(),
        .row = start_row,
        .col = start_col,
      });

      goto CONTINUE;
    }


    if (auto simple_token = simple_tokens.find(c); simple_token != simple_tokens.end()) {
      out.push_back(Token{ 
        .type = simple_token->second,
        .row = row,
        .col = col,
      });
      goto CONTINUE;
    }

    if (is_identifier_char(c)) {
      size_t start_row = row;
      size_t start_col = col;
      std::stringstream ss;
      char it_c = c;
      for (;;) {
        ss << it_c;
        if (sv.length() == i + 1) break;
        if (!is_identifier_char(sv[i + 1])) break;
        it_c = sv[inc_index()];
      }
      out.push_back(Token{
        .type = TokenType::IDENTIFIER,
        .str_val = ss.str(),
        .row = start_row,
        .col = start_col,
      });
    }

  CONTINUE:
    inc_index();
  }

  return { out, errors };
}


std::ostream& operator<<(std::ostream& out, const Token& token) {
  out << magic_enum::enum_name(token.type);
  if (
      token.type == TokenType::STRING_LITERAL
      || token.type == TokenType::INT_LITERAL
      || token.type == TokenType::IDENTIFIER
    ) {
      out << ": \"" << token.str_val << "\"";
    }
  return out;
}

void stringify_tokens(const std::vector<Token> tokens, std::stringstream& ss) {
  ss << std::setfill('0');
  ss << "[\n";
  size_t width = static_cast<size_t>(std::log10(tokens.size())) + 1;
  for (size_t i = 0; i < tokens.size(); ++i) {
    const Token& token = tokens[i];
    ss << "  " << std::setw(width) << i << ". " << token;
    ss << " - " << (token.row + 1) << ":" << (token.col + 1);
    ss << "\n";
  }
  ss << "]";
}
void stringify_lexer_errors(const std::vector<LexerError> errors, std::stringstream& ss) {
  for (size_t i = 0; i< errors.size(); ++i) {
    const auto& error = errors[i];
    ss << error.message << ":" << (error.row + 1) << ":" << (error.col + 1);
    ss << "\n";
  }
}
