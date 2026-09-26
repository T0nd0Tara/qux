#pragma once
#include <sstream>

#include "ast.hpp"
#include "typing.hpp"


void generate_ir(const AstNode& ast, std::stringstream& ss);
