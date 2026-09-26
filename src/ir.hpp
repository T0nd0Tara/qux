#pragma once
#include <sstream>

#include "ast.hpp"

struct JitResult {
  int return_code = 0;
  bool compilation_succeeded = true;
};

void generate_ir(const AstNode& ast, std::stringstream& ss);
bool write_machine_code(const std::string& ir, std::string file_name);
JitResult create_jit_run_func(const std::string& ir);
