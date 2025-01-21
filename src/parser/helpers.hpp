#pragma once

#include <ostream>
#include <sstream>
#include "ast_types.hpp"

namespace ast {

void get_tree(std::ostream& ss, const Node* node, const std::string prefix, bool is_last)
{
    if(node == nullptr) return;

    ss << (is_last ? "└──" : "├──");

    // print the value of the node
    ss << typeid(*node).name() << '\n';

    // enter the next tree level - left and right branch
    std::string next_prefix = prefix + (is_last ? "    " : "│   ");

    const std::vector<Node*> children = node->get_children();

    for (size_t i=0; i < children.size(); i++) {
      get_tree(ss, children[i], next_prefix, i + 1 == children.size());

    }
}

inline void get_tree(std::ostream& ss, const Node* node) {
    get_tree(ss, node, "", true);
}

}

