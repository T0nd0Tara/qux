#pragma once
#include <string>

enum class TokenType {
    variable,
    decleration,
    runtime_assignment,
    comtime_assignment,
    typing,

    string_literal,
    number_literal,

    brace_open,
    brace_close,

    scope_open,
    scope_close,

    end_statement,
};

struct Token {
  TokenType type;
  std::string value = "";
};
