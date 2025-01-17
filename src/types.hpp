#pragma once
#include <string>

enum class TokenType {
    variable,
    decleration,
    runtime_assignment,
    comtime_assignment,
    typing,
};

struct Token {
  TokenType type;
  std::string value = "";
};
