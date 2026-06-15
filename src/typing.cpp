#include "typing.hpp"
#include "ast.hpp"
#include <cassert>


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
