#include "ir.hpp"
#include "ast.hpp"
#include "typing.hpp"
#include <cassert>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <functional>

std::unordered_map<std::string, Typing*> typing_map = {};

void typing_to_ir(const Typing* typing, std::string_view variable_name, std::stringstream& ss) {
  assert(typing != nullptr);

  switch (typing->type) {
    case TypingType::I8:  ss << "int8_t";  return;
    case TypingType::I16: ss << "int16_t"; return;
    case TypingType::I32: ss << "int32_t"; return;
    case TypingType::I64: ss << "int64_t"; return;

    case TypingType::U8:  ss << "uint8_t";  return;
    case TypingType::U16: ss << "uint16_t"; return;
    case TypingType::U32: ss << "uint32_t"; return;
    case TypingType::U64: ss << "uint64_t"; return;

    case TypingType::F32: ss << "float"; return;
    case TypingType::F64: ss << "double"; return;

    case TypingType::FUNC:
      typing_to_ir(typing->return_type, "", ss);
      ss << " ";
      ss << variable_name;
      ss << "(";
      for (const auto& param_type: typing->parameters_types) {
        typing_to_ir(param_type, "", ss);
      }
      ss << ")";
      
      return;
  }
}
void generate_decleration(const AstNode& ast, std::stringstream& ss) {
  assert(ast.type == AstNodeType::DECLARE);

  const auto& variable_node = ast.children[0];

  assert(variable_node.type == AstNodeType::VARIABLE);

  typing_map[variable_node.variable_name] = variable_node.typing;
  typing_to_ir(variable_node.typing, variable_node.variable_name, ss);
  ss << ";\n";
}
void generate_expr_ir(const AstNode& ast, std::stringstream &ss) {
  switch (ast.type) {
    case AstNodeType::INT_LITERAL: {
      ss << ast.str_val;
      return;
    }
    case AstNodeType::STRING_LITERAL: {
      ss << '"' << ast.str_val << '"';
      return;
    }
  }
}

void generate_assignment(const AstNode& ast, std::stringstream& ss);
void hoist_all_declerations(const AstNode& ast, std::stringstream& ss);

void generate_scope_ir(const AstNode& ast, std::stringstream &ss) {
  hoist_all_declerations(ast, ss);

  for (const auto& node: ast.children) {
    switch (node.type){
      case AstNodeType::ASSIGN_COM: generate_assignment(node, ss); break;
      case AstNodeType::FUNC_CALL: {
        const auto& variable = node.children[0];
        assert(variable.type == AstNodeType::VARIABLE);

        const auto& args = node.children[1];
        assert(args.type == AstNodeType::ARGS);

        ss << variable.variable_name;
        ss << "(";
        for (size_t i = 0; i < args.children.size(); i++) {
          const auto& arg = args.children[i];
          generate_expr_ir(arg, ss);
          if (i < args.children.size() - 1) ss << ", ";
        }
        ss << ");\n";

        break;
      }
      case AstNodeType::RETURN: {
        ss << "return ";
        generate_expr_ir(node.children[0], ss);
        ss << ";\n";
        break;
      }
    }
  }
}

void generate_assignment(const AstNode& ast, std::stringstream& ss) {
  assert(ast.type == AstNodeType::ASSIGN_COM || ast.type == AstNodeType::ASSIGN_RUN);
  const auto& variable_node = ast.children[0];
  const Typing* typing = typing_map[variable_node.variable_name];
  assert(typing != nullptr);

  typing_to_ir(typing, variable_node.variable_name, ss);

  const auto& rvalue_node = ast.children[1];
  if (rvalue_node.type != AstNodeType::FUNC) {
    ss << " = ";
    ss << ";";
    return;
  }

  ss << "{\n";
  generate_scope_ir(rvalue_node, ss);
  ss << "\n}";
}

void hoist_all_declerations(const AstNode& ast, std::stringstream& ss) {
  for (const auto& node : ast.children) {
    if (node.type == AstNodeType::DECLARE)
      generate_decleration(node, ss);
  }
}

void generate_ir(const AstNode& ast, std::stringstream& ss) {
  assert(ast.type == AstNodeType::ROOT);
  
  ss << "#include <stdio.h>\n";
  ss << "#include <stdint.h>\n";

  ss << "#define print printf\n";

  generate_scope_ir(ast, ss);  
}

