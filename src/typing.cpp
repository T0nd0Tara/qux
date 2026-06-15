#include "typing.hpp"
#include "ast.hpp"
#include <cassert>
#include <sstream>
#include <magic_enum/magic_enum.hpp>


void fill_typing(AstNode& ast) {
  assert(ast.type == AstNodeType::ROOT);

  for (auto& node : ast.children) {

    if (node.type != AstNodeType::DECLARE) continue;
    if (node.typing != nullptr) continue;

    auto& var = node.children[0]; 
    if (var.variable_name == "main") {
      Typing* return_type = new Typing {
        .type = TypingType::U8
      };
      var.typing = new Typing{
        .type = TypingType::FUNC,
        .return_type = return_type,
      };
    }
  }
}

void stringify_typing(const Typing* typing, std::stringstream& ss) {
  if (typing == nullptr) return;

  if (typing->pointer_to) {
    ss << "*";
    stringify_typing(typing->pointer_to, ss);
    return;
  }

  if (typing->type == TypingType::FUNC) {
    ss << "(";
    const auto& params = typing->parameters_types;
    for (size_t param_idx = 0; param_idx < params.size(); ++param_idx) {
      stringify_typing(params[param_idx], ss);
      if (param_idx < params.size() - 1) {
        ss << ", ";
      }
    }
    ss << ") -> (";
    stringify_typing(typing->return_type, ss);
    ss << ")";
    return;
  }

  ss << magic_enum::enum_name(typing->type);
}
std::string stringify_typing(const Typing* typing) {
  std::stringstream ss;
  stringify_typing(typing, ss);
  return ss.str();
}
