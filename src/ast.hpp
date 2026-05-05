#pragma once
#include "lexer.hpp"
#include <sstream>


enum class AstNodeType {
  NOOP,

  ROOT,

  INT_LITERAL,
  STRING_LITERAL,

  ARGS,

  FUNC,

  FUNC_CALL,

  ASSIGN_RUN,
  ASSIGN_COM,
  DECLARE,

  VARIABLE,

  RETURN,
};
struct AstNode {
  AstNodeType type;

  std::vector<AstNode> children = {};
  std::string variable_name = "";
  std::string str_val = "";
};

struct AstError {
  size_t row, col;
  std::string message;
};


std::pair<AstNode, std::vector<AstError>> get_ast(const std::vector<Token>& tokens);
void stringify_ast(const AstNode& node, std::stringstream& ss);
void stringify_ast_errors(const std::vector<AstError>& errors, std::stringstream& ss);
