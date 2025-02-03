#pragma once

#include "../variable_types.hpp"
#include <memory>
#include <vector>
namespace ast {
enum class NodeType {
  root,
  variable,
  function_literal,
};

struct Parameter {
  VariableType type;
  std::string name;
};

struct Node;

struct FunctionData {
  std::vector<Parameter> parameters;
  VariableType return_type;
  std::vector<std::unique_ptr<Node>> body;
};



struct Variable {
  VariableType type;
  std::string name;
  union {
    int i;
    float f;
    FunctionData function;

  } value;
};

struct Node {
  NodeType type;

  union {
    std::vector<Node> nodes; // for NodeType::root
    Variable variable; // for NodeType::variable
  } node_data;
};

}

