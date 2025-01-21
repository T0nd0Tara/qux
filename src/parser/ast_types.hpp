#pragma once
#include <memory>
#include <vector>

namespace ast {
struct Node {
  virtual std::vector<Node*> get_children() const = 0;
};

typedef std::vector<std::unique_ptr<Node>> Nodes;

struct Root : public Node 
{
  Nodes nodes;
  
  std::vector<Node*> get_children() const 
  {
    std::vector<Node*> out(nodes.size());

    for (const auto& node : nodes)
      out.push_back(node.get());

    return out;
  }
};

struct Type : public Node {
  std::vector<Node*> get_children() const 
  {
    std::vector<Node*> out;
    return out;
  }
};



struct Variable : public Node {
  std::vector<Node*> get_children() const 
  {
    std::vector<Node*> out;
    return out;
  }

};

struct Parameter : public Variable {
};

typedef std::vector<std::unique_ptr<Parameter>> Parameters;

struct Function : public Variable 
{
  Parameters params;
  Nodes body;

  std::unique_ptr<Type> return_type;

  std::vector<Node*> get_children() const 
  {
    std::vector<Node*> out(params.size() + body.size() + 1); // +1 for the return type
    for (const auto& node : params) out.push_back(node.get());
    for (const auto& node : body) out.push_back(node.get());

    out.push_back(return_type.get());

    return out;
  }
};

struct Assignment : Node {
  std::unique_ptr<Variable> variable;
  std::unique_ptr<Type> type;
  bool compile_type;

  std::vector<Node*> get_children() const 
  {
    return std::vector<Node*> {
      variable.get(), type.get()
    };
  }
};

struct Call : Node {
  std::unique_ptr<Function> variable;

  std::vector<Node*> get_children() const 
  {
    return std::vector<Node*> {
      variable.get(),
    };
  }
};

}


