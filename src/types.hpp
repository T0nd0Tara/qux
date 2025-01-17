#pragma once
#include <string>

enum class TokenType {
    variable,
    decleration,
    runtime_assignment,
    comtime_assignment,
    typing,

    brace_open,
    brace_close,

    scope_open,
    scope_close,
};

struct Token {
  TokenType type;
  std::string value = "";
};
